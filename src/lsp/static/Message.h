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

/**
 * @struct ResponseError
 * @brief Represents an error response in the LSP.
 * This implementation is slightly off-spec -- the spec says that ResponseError
 * is an **optional** field in a ResponseMessage, but our implementation always
 * includes it, and makes the distinction that if the code is 0, there is no error.
 * We should definitely change this to be more exactly in-line with the spec, but
 * this has worked for us so far.
 * 
 */
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

struct NotificationMessageBase : Message {
	std::string method;
	// Notifications do not have an ID
	virtual ~NotificationMessageBase() = default;
};

template<typename ParamsType>
struct RequestMessage;

template <typename ResultType>
struct ResponseMessage;

template<typename ParamsType>
struct NotificationMessage;

/**
 * @struct GenericRequestMessage
 * @brief A generic type that can be converted to/from any specific RequestMessage type.
 * 
 */
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

	/**
	 * @brief Convert this GenericRequestMessage to a specific RequestMessage type.
	 *
	 * This method allows converting the generic message to a specific type based on the ParamsType.
	 * It uses the JSON serialization/deserialization to convert the params field to the specific type.
	 * 
	 * @tparam ParamsType The specific parameters type to convert to.
	 * @return RequestMessage<ParamsType> The specific RequestMessage type with the converted parameters.
	 */
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

/**
 * @struct GenericResponseMessage
 * @brief A generic type that can be converted to/from any specific ResponseMessage type.
 * 
 */
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

/**
 * @struct GenericNotificationMessage
 * @brief A generic type that can be converted to/from any specific NotificationMessage type.
 * 
 */
struct GenericNotificationMessage : public NotificationMessageBase {
	LSPAny params;

	GenericNotificationMessage() = default;

	template<typename ParamsType>
	GenericNotificationMessage(const NotificationMessage<ParamsType>& msg) {
		jsonrpc = msg.jsonrpc;
		method = msg.method;
		nlohmann::json j;
		nlohmann::adl_serializer<ParamsType>::to_json(j, msg.params);
		params = j.get<LSPAny>(); // Store params as LSPAny
	}

	friend void to_json(nlohmann::json& j, const GenericNotificationMessage& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["method"] = msg.method;
		j["params"] = msg.params; // Serialize params as LSPAny
	}

	friend void from_json(const nlohmann::json& j, GenericNotificationMessage& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("method").get_to(msg.method);
		if (j.contains("params")) {
			msg.params = j.at("params").get<LSPAny>();
		} else {
			msg.params = nullptr; // Default to null if not present
		}
	}

	template <typename ParamsType>
	NotificationMessage<ParamsType> toSpecific() const {
		NotificationMessage<ParamsType> specific;
		specific.jsonrpc = jsonrpc;
		specific.method = method;
		nlohmann::json j;
		nlohmann::adl_serializer<LSPAny>::to_json(j, params);
		specific.params = j.get<ParamsType>(); // Convert JSON to ParamsType
		return specific;
	}

	template <typename ParamsType>
	static GenericNotificationMessage fromSpecific(const NotificationMessage<ParamsType>& specific) {
		GenericNotificationMessage generic;
		generic.jsonrpc = specific.jsonrpc;
		generic.method = specific.method;
		nlohmann::json j;
		nlohmann::adl_serializer<ParamsType>::to_json(j, specific.params);
		generic.params = j.get<LSPAny>(); // Store params as LSPAny
		return generic;
	}
};

template <typename ParamsType>
struct RequestTraits;

/**
 * @struct RequestMessage
 * @brief A template class for all LSP request messages.
 *
 * Different request types are distinguished by their method name and parameters.
 * This class is templated on the ParamsType, which represents the specific parameters
 * for the request. It inherits from RequestMessageBase to provide the common structure.
 * The method name is determined by the RequestTraits specialization for the ParamsType.
 * 
 * @tparam ParamsType 
 */
template<typename ParamsType>
struct RequestMessage : public RequestMessageBase {
	ParamsType params;

	RequestMessage() {
		static_assert(std::is_base_of<RequestTraits<ParamsType>, RequestTraits<ParamsType>>::value,
			"No RequestTraits specialization found for this ParamsType");
		method = RequestTraits<ParamsType>::method; // Set method from traits
	}

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

/**
 * @struct ResponseMessage
 * @brief A template class for all LSP response messages.
 *
 * Different response types are distinguished by their result type.
 * This class is templated on the ResultType, which represents the specific result
 * for the response. It inherits from ResponseMessageBase to provide the common structure.
 * The error field is included to indicate if there was an error processing the request.
 * 
 * @tparam ResultType 
 */
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

	/**
	 * @brief Convert this ResponseMessage to a GenericResponseMessage.
	 * 
	 * @return GenericResponseMessage 
	 */
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

template <typename ParamsType>
struct NotificationTraits;

/**
 * @struct NotificationMessage
 * @brief A template class for all LSP notification messages.
 *
 * Notifications do not have a response, and are distinguished by their method name and parameters.
 * This class is templated on the ParamsType, which represents the specific parameters of the notification.
 * It inherits from NotificationMessageBase to provide the common structure.
 * The method name is determined by the NotificationTraits specialization for the ParamsType.
 *
 * Notifications in the LSP spec are used to send information (either from the server to the client, or vice versa)
 * without expecting a response. They are typically used for events or updates that do not require a reply.
 * 
 * @tparam ParamsType 
 */
template<typename ParamsType>
struct NotificationMessage : public NotificationMessageBase {
	ParamsType params;

	NotificationMessage() {
		static_assert(std::is_base_of<NotificationTraits<ParamsType>, NotificationTraits<ParamsType>>::value,
			"No NotificationTraits specialization found for this ParamsType");
		method = NotificationTraits<ParamsType>::method; // Set method from traits
	}

	friend void to_json(nlohmann::json& j, const NotificationMessage<ParamsType>& msg) {
		j = nlohmann::json::object();
		j["jsonrpc"] = msg.jsonrpc;
		j["method"] = msg.method;
		j["params"] = msg.params;
	}

	friend void from_json(const nlohmann::json& j, NotificationMessage<ParamsType>& msg) {
		j.at("jsonrpc").get_to(msg.jsonrpc);
		j.at("method").get_to(msg.method);
		if (j.contains("params")) {
			msg.params = j.at("params").get<ParamsType>();
		} else {
			msg.params = ParamsType(); // Default to empty object if not present
		}
	}

	/**
	 * @brief Convert this NotificationMessage to a GenericNotificationMessage.
	 * 
	 * @return GenericNotificationMessage 
	 */
	GenericNotificationMessage toGeneric() const {
		GenericNotificationMessage generic;
		generic.jsonrpc = jsonrpc;
		generic.method = method;
		nlohmann::json j;
		nlohmann::adl_serializer<ParamsType>::to_json(j, params);
		generic.params = j.get<LSPAny>(); // Store params as LSPAny
		return generic;
	}
};
