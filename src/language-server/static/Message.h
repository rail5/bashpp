#pragma once
#include <string>
#include <variant>
#include <nlohmann/json.hpp>
#include "LSPTypes.h"

struct ResponseError {
	int code;
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

template<typename ParamsType>
struct RequestMessageT : public Message {
	std::variant<int, std::string> id;
	std::string method;
	ParamsType params;

	friend void to_json(nlohmann::json& j, const RequestMessageT<ParamsType>& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["id"] = msg.id;
		j["method"] = msg.method;
		j["params"] = msg.params;
	}

	friend void from_json(const nlohmann::json& j, RequestMessageT<ParamsType>& msg) {
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

using RequestMessage = RequestMessageT<LSPAny>; // Not strictly correct according to the spec
	// According to the spec, params should be either an array or an object
	// LSPAny covers these but also includes primitive types, which are not allowed

template<typename ResultType>
struct ResponseMessageT : public Message {
	std::variant<int, std::string, std::nullptr_t> id;
	ResultType result;
	ResponseError error;

	friend void to_json(nlohmann::json& j, const ResponseMessageT<ResultType>& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["id"] = msg.id;
		if (msg.error.code != 0) {
			j["error"] = msg.error;
		} else {
			j["result"] = msg.result;
		}
	}

	friend void from_json(const nlohmann::json& j, ResponseMessageT<ResultType>& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("id").get_to(msg.id);
		if (j.contains("error")) {
			msg.error = j.at("error").get<ResponseError>();
		} else {
			msg.result = j.at("result").get<ResultType>();
			msg.error = {0, "", nullptr}; // Default error if not present
		}
	}
};

using ResponseMessage = ResponseMessageT<LSPAny>;

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
