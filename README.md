# Bash++

Bash with classes

## The Basic Idea

This is intended to be a source-to-source compiler.

A Bash++ script will be converted into ordinary Bash for execution

## Syntax

 - The Bash++ syntax will be a superset of the Bash syntax

 - The syntax will be designed to be easily converted to ordinary Bash script

Here is some (rather silly/useless) example Bash++ code:

```sh
@class Bool {
	@private value="false"

	@public @method get {
		echo "@value"
	}

	@public @method set input_value {
		if [[ @input_value == "true" ]] || [[ @input_value -eq 1 ]]; then
			@value="true"
		else
			@value="false"
		fi
	}
	
	@constructor input_value {
		@this.set "$input_value"
	}
}

@class File {
	@private file_path
	@private @Bool file_exists

	@constructor {
		@file_path=""
		@file_exists.set false
	}

	@constructor input_file_path {
		@file_path="$input_file_path"
		@file_exists.set $([[ -f "@file_path" ]] && echo "true" || echo "false")
	}

	@public @method read {
		if [[ @file_exists.get == "true" ]]; then
			cat "@file_path"
		fi
	}

	@public @method append input_string {
		echo "$input_string" >> "@file_path"
	}
}

@File new_file "./new-file"
@new_file.read

@new_file.append "Hello from Bash++!"
@new_file.read

@File old_file "./old-file"
for word in @old_file.read; do
	echo "Word: $word"
done
```
