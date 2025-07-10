/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * 
 * This file is part of a C++ implementation of the Language Server Protocol (LSP) types.
 */

#pragma once
#include <string>
#include <variant>
#include <nlohmann/json.hpp>
#include "LSPTypes.h"

struct ResponseError {
	int code = 0; // Error code, 0 means no error
	std::string message;
	LSPAny data; // Optional additional data

	friend void to_json(nlohmann::json& j, const ResponseError& error) {
		j = nlohmann::json::object();
		j["code"] = error.code;
		j["message"] = error.message;
		if (error.data.get_if<std::nullptr_t>()) {
			j["data"] = nullptr; // Explicitly set to null if data is nullptr
		} else {
			j["data"] = error.data; // Serialize the data if it's not null
		}
	}

	friend void from_json(const nlohmann::json& j, ResponseError& error) {
		j.at("code").get_to(error.code);
		j.at("message").get_to(error.message);
		if (j.contains("data")) {
			error.data = j.at("data").get<LSPAny>();
		} else {
			error.data = nullptr; // Default to null if not present
		}
	}
};

struct Message {
	std::string jsonrpc = "2.0";
};

struct RequestMessageBase : Message {
	std::variant<int, std::string, std::nullptr_t> id;
	std::string method;
	virtual ~RequestMessageBase() = default;
};

struct ResponseMessageBase : Message {
	std::variant<int, std::string, std::nullptr_t> id;
	ResponseError error; // Error information if any
	virtual ~ResponseMessageBase() = default;
};

template<typename ParamsType>
struct RequestMessage : public RequestMessageBase {
	ParamsType params;

	friend void to_json(nlohmann::json& j, const RequestMessage<ParamsType>& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["id"] = msg.id;
		j["method"] = msg.method;
		j["params"] = msg.params;
	}

	friend void from_json(const nlohmann::json& j, RequestMessage<ParamsType>& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("id").get_to(msg.id);
		j.at("method").get_to(msg.method);
		if (j.contains("params")) {
			msg.params = j.at("params").get<ParamsType>();
		} else {
			msg.params = ParamsType(); // Default to empty object if not present
		}
	}
};

template <typename ResultType>
struct ResponseMessage;

struct GenericRequestMessage : public RequestMessageBase {
	LSPAny params;

	GenericRequestMessage() = default;

	template<typename ParamsType>
	GenericRequestMessage(const RequestMessage<ParamsType>& msg) {
		jsonrpc = msg.jsonrpc;
		id = msg.id;
		method = msg.method;
		nlohmann::json j;
		nlohmann::adl_serializer<ParamsType>::to_json(j, msg.params);
		params = j.get<LSPAny>(); // Store params as LSPAny
	}

	friend void to_json(nlohmann::json& j, const GenericRequestMessage& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["id"] = msg.id;
		j["method"] = msg.method;
		j["params"] = msg.params; // Serialize params as LSPAny
	}

	friend void from_json(const nlohmann::json& j, GenericRequestMessage& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("id").get_to(msg.id);
		j.at("method").get_to(msg.method);
		if (j.contains("params")) {
			msg.params = j.at("params").get<LSPAny>();
		} else {
			msg.params = nullptr; // Default to null if not present
		}
	}

	template <typename ParamsType>
	RequestMessage<ParamsType> toSpecific() const {
		RequestMessage<ParamsType> specific;
		specific.jsonrpc = jsonrpc;
		specific.id = id;
		specific.method = method;

		nlohmann::json j;
		nlohmann::adl_serializer<LSPAny>::to_json(j, params);
		specific.params = j.get<ParamsType>(); // Convert JSON to ParamsType

		return specific;
	}
};

struct GenericResponseMessage : public ResponseMessageBase {
	LSPAny result;

	GenericResponseMessage() = default;

	template<typename ResultType>
	GenericResponseMessage(const ResponseMessage<ResultType>& msg) {
		jsonrpc = msg.jsonrpc;
		id = msg.id;
		if (msg.error.code != 0) {
			error = msg.error; // Store error if present
		} else {
			nlohmann::json j;
			nlohmann::adl_serializer<ResultType>::to_json(j, msg.result);
			result = j.get<LSPAny>(); // Store result as LSPAny
		}
	}

	friend void to_json(nlohmann::json& j, const GenericResponseMessage& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["id"] = msg.id;
		if (msg.error.code != 0) {
			j["error"] = msg.error; // Serialize error if present
		} else {
			j["result"] = msg.result; // Serialize result as LSPAny
		}
	}

	friend void from_json(const nlohmann::json& j, GenericResponseMessage& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("id").get_to(msg.id);
		if (j.contains("error")) {
			msg.error = j.at("error").get<ResponseError>();
			msg.result = nullptr; // Default result if error is present
		} else {
			msg.result = j.at("result").get<LSPAny>();
			msg.error = {0, "", nullptr}; // Default error if not present
		}
	}

	template <typename ResultType>
	static GenericResponseMessage fromSpecific(const ResponseMessage<ResultType>& specific) {
		GenericResponseMessage generic;
		generic.jsonrpc = specific.jsonrpc;
		generic.id = specific.id;

		if (specific.error.code != 0) {
			generic.error = specific.error; // Store error if present
		} else {
			generic.result = specific.result; // Store result as LSPAny
		}

		return generic;
	}
};

template<typename ResultType>
struct ResponseMessage : public ResponseMessageBase {
	ResponseError error;
	ResultType result;

	friend void to_json(nlohmann::json& j, const ResponseMessage<ResultType>& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["id"] = msg.id;
		if (msg.error.code != 0) {
			j["error"] = msg.error;
		} else {
			j["result"] = msg.result;
		}
	}

	friend void from_json(const nlohmann::json& j, ResponseMessage<ResultType>& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("id").get_to(msg.id);
		if (j.contains("error")) {
			msg.error = j.at("error").get<ResponseError>();
		} else {
			msg.result = j.at("result").get<ResultType>();
			msg.error = {0, "", nullptr}; // Default error if not present
		}
	}

	GenericResponseMessage toGeneric() const {
		GenericResponseMessage generic;
		generic.jsonrpc = jsonrpc;
		generic.id = id;

		if (error.code != 0) {
			generic.error = error; // Store error if present
		} else {
			nlohmann::json j;
			nlohmann::adl_serializer<ResultType>::to_json(j, result);
			generic.result = j.get<LSPAny>(); // Store result as LSPAny
		}

		return generic;
	}
};

struct NotificationMessage : public Message {
	std::string method;
	nlohmann::json params;

	friend void to_json(nlohmann::json& j, const NotificationMessage& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["method"] = msg.method;
		j["params"] = msg.params;
	}

	friend void from_json(const nlohmann::json& j, NotificationMessage& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("method").get_to(msg.method);
		if (j.contains("params")) {
			msg.params = j.at("params");
		} else {
			msg.params = nlohmann::json::object(); // Default to empty object if not present
		}
	}
};
