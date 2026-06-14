#!/usr/bin/env python3

# Copyright (C) 2026 Andrew S. Rightenburg
# Bash++: Bash with classes
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import pathlib
import subprocess
import sys
import tempfile
import time


SOURCE = """@class Parent {
\t@public inherited=0
}

@class Thing {
\t@public value=0
\t@public @method act argument {
\t\techo "$argument"
\t}
}

@class Child : Parent {
\t@public member=1
\t@public @Thing* pointerMember
\t@public @method run primitive @Thing* item {
\t\t@Thing object
\t\t@Thing* pointer
\t\t@Thing* allocated=@new Thing
\t\t@object.value=primitive
\t\t@object.act primitive
\t\t@this.member=@object.value
\t\techo "😀"; @Thing unicodeObject
\t\techo "@object.value @(echo @object.value)"
\t\techo @dynamic_cast<Thing> &@object
\t}
}

@Child child
@child.run argument @nullptr
"""

MALFORMED_SOURCE = """@class Broken {
\t@public value=
\t@public @method incomplete
"""


class LanguageServerClient:
	def __init__(self, executable):
		self.next_request_id = 1
		self.process = subprocess.Popen(
			[executable, "--stdio", "-j1"],
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
		)

	def send(self, message):
		payload = json.dumps(message, separators=(",", ":")).encode()
		frame = f"Content-Length: {len(payload)}\r\n\r\n".encode() + payload
		self.process.stdin.write(frame)
		self.process.stdin.flush()

	def read_message(self):
		content_length = None
		while True:
			line = self.process.stdout.readline()
			if not line:
				stderr = self.process.stderr.read().decode(errors="replace")
				raise RuntimeError(f"language server exited unexpectedly:\n{stderr}")
			if line in (b"\r\n", b"\n"):
				break
			name, value = line.decode().split(":", 1)
			if name.lower() == "content-length":
				content_length = int(value.strip())

		if content_length is None:
			raise RuntimeError("language server response omitted Content-Length")
		return json.loads(self.process.stdout.read(content_length))

	def request(self, method, params):
		request_id = self.next_request_id
		self.next_request_id += 1
		self.send({
			"jsonrpc": "2.0",
			"id": request_id,
			"method": method,
			"params": params,
		})

		while True:
			response = self.read_message()
			if response.get("id") == request_id:
				if "error" in response:
					raise RuntimeError(f"{method} failed: {response['error']}")
				return response["result"]

	def notify(self, method, params):
		self.send({
			"jsonrpc": "2.0",
			"method": method,
			"params": params,
		})

	def close(self):
		try:
			self.request("shutdown", None)
			self.notify("exit", None)
		finally:
			self.process.stdin.close()
			self.process.wait(timeout=10)


def file_uri(path):
	return pathlib.Path(path).resolve().as_uri()


def utf16_slice(line, start, length):
	encoded = line.encode("utf-16-le")
	start_byte = start * 2
	end_byte = (start + length) * 2
	return encoded[start_byte:end_byte].decode("utf-16-le")


def decode_tokens(source, legend, encoded_tokens):
	if len(encoded_tokens) % 5 != 0:
		raise AssertionError("semantic token data length is not divisible by five")

	lines = source.splitlines()
	tokens = []
	line = 0
	start = 0
	for index in range(0, len(encoded_tokens), 5):
		delta_line, delta_start, length, token_type, modifiers = encoded_tokens[index:index + 5]
		line += delta_line
		start = delta_start if delta_line else start + delta_start
		text = utf16_slice(lines[line], start, length)
		tokens.append({
			"text": text,
			"type": legend["tokenTypes"][token_type],
			"declaration": bool(modifiers & 1),
			"line": line,
			"start": start,
		})
	return tokens


def assert_token(tokens, text, token_type, declaration):
	if not any(
		token["text"] == text
		and token["type"] == token_type
		and token["declaration"] == declaration
		for token in tokens
	):
		raise AssertionError(
			f"missing semantic token {text!r} ({token_type}, declaration={declaration})"
		)


def request_tokens(client, uri, source, legend):
	result = client.request(
		"textDocument/semanticTokens/full",
		{"textDocument": {"uri": uri}},
	)
	if result is None:
		raise AssertionError("semantic token request unexpectedly returned null")
	return decode_tokens(source, legend, result["data"])


def main():
	executable = sys.argv[1] if len(sys.argv) > 1 else "bin/bpp-lsp"

	with tempfile.TemporaryDirectory(prefix="bashpp-lsp-test-") as temporary_directory:
		valid_path = pathlib.Path(temporary_directory) / "semantic-tokens.bpp"
		malformed_path = pathlib.Path(temporary_directory) / "malformed.bpp"
		valid_path.write_text(SOURCE)
		malformed_path.write_text(MALFORMED_SOURCE)

		client = LanguageServerClient(executable)
		try:
			initialize_result = client.request("initialize", {
				"processId": None,
				"rootUri": file_uri(temporary_directory),
				"capabilities": {
					"general": {"positionEncodings": ["utf-16"]},
					"textDocument": {
						"semanticTokens": {
							"requests": {"full": True},
							"tokenTypes": [],
							"tokenModifiers": [],
							"formats": ["relative"],
						}
					},
				},
			})

			provider = initialize_result["capabilities"]["semanticTokensProvider"]
			legend = provider["legend"]
			assert legend["tokenTypes"] == [
				"class",
				"method",
				"property",
				"variable",
				"parameter",
			]
			assert legend["tokenModifiers"] == ["declaration"]
			assert provider["full"] is True

			valid_uri = file_uri(valid_path)
			client.notify("textDocument/didOpen", {
				"textDocument": {
					"uri": valid_uri,
					"languageId": "bashpp",
					"version": 1,
					"text": SOURCE,
				}
			})

			tokens = request_tokens(client, valid_uri, SOURCE, legend)
			for token in [
				("Parent", "class", True),
				("Parent", "class", False),
				("Thing", "class", True),
				("Child", "class", True),
				("act", "method", True),
				("act", "method", False),
				("run", "method", True),
				("run", "method", False),
				("member", "property", True),
				("member", "property", False),
				("pointerMember", "property", True),
				("value", "property", True),
				("value", "property", False),
				("primitive", "parameter", True),
				("item", "parameter", True),
				("object", "variable", True),
				("object", "variable", False),
				("pointer", "variable", True),
				("allocated", "variable", True),
				("unicodeObject", "variable", True),
				("child", "variable", True),
				("child", "variable", False),
			]:
				assert_token(tokens, *token)

			thing_references = [
				token for token in tokens
				if token["text"] == "Thing"
				and token["type"] == "class"
				and not token["declaration"]
			]
			if len(thing_references) < 7:
				raise AssertionError("class references from declarations, new, and casts are missing")

			nested_value_references = [
				token for token in tokens
				if token["text"] == "value"
				and token["type"] == "property"
				and not token["declaration"]
			]
			if len(nested_value_references) < 4:
				raise AssertionError("property references inside strings or supershells are missing")

			unicode_token = next(
				token for token in tokens
				if token["text"] == "unicodeObject" and token["declaration"]
			)
			expected_utf16_column = len('\t\techo "😀"; @Thing '.encode("utf-16-le")) // 2
			assert unicode_token["start"] == expected_utf16_column

			changed_source = SOURCE + "\n@Thing changedObject\n"
			client.notify("textDocument/didChange", {
				"textDocument": {"uri": valid_uri, "version": 2},
				"contentChanges": [{"text": changed_source}],
			})

			deadline = time.monotonic() + 5
			while True:
				changed_tokens = request_tokens(client, valid_uri, changed_source, legend)
				if any(token["text"] == "changedObject" for token in changed_tokens):
					break
				if time.monotonic() >= deadline:
					raise AssertionError("semantic tokens did not reflect unsaved changes")
				time.sleep(0.1)
			assert_token(changed_tokens, "changedObject", "variable", True)

			malformed_uri = file_uri(malformed_path)
			client.notify("textDocument/didOpen", {
				"textDocument": {
					"uri": malformed_uri,
					"languageId": "bashpp",
					"version": 1,
					"text": MALFORMED_SOURCE,
				}
			})
			request_tokens(client, malformed_uri, MALFORMED_SOURCE, legend)
		finally:
			client.close()

	print("Bash++ semantic token integration tests passed.")


if __name__ == "__main__":
	main()
