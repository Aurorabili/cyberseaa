#ifndef ERROR_HPP_
#define ERROR_HPP_
#include <system_error>

namespace Core {
namespace WebSocket {
    enum class Error;
    enum class Condition;
}
}

namespace std {
template <>
struct is_error_code_enum<Core::WebSocket::Error> {
    static bool const value = true;
};
template <>
struct is_error_condition_enum<Core::WebSocket::Condition> {
    static bool const value = true;
};
} // std

namespace Core {
namespace WebSocket {
    enum class Error {
        BUFFER_OVERFLOW = 800000,
        INVALID_READ_OPERATION,

        READ_MESSAGE_TOO_BIG, // The socket message exceeded the locally configured limit
        WRITE_MESSAGE_TOO_BIG, // The socket message exceeded the locally configured limit
        READ_TIMEOUT, // socket read time out
        SEND_QUEUE_TOO_BIG, // send message queue size exceeded config.h configured limit
        WS_BAD_HTTP_HEADER, // websocket parse http header failed
        WS_BAD_HTTP_VERSION, // The WebSocket handshake was not HTTP/1.1
        WS_BAD_METHOD, // The WebSocket handshake method was not GET
        WS_BAD_HTTP_STATUS_CODE, // The WebSocket handshake http status code was not 101
        WS_NO_HOST, // The WebSocket handshake Host field is missing
        WS_NO_CONNECTION, // The WebSocket handshake Connection field is missing
        WS_NO_CONNECTION_UPGRADE, // The WebSocket handshake Connection field is missing the upgrade token
        WS_NO_UPGRADE, // The WebSocket handshake Upgrade field is missing
        WS_NO_UPGRADE_WEBSOCKET, // The WebSocket handshake Upgrade field is missing the websocket token
        WS_NO_SEC_KEY, // The WebSocket handshake Sec-WebSocket-Key field is missing
        WS_BAD_SEC_KEY, // The WebSocket handshake Sec-WebSocket-Key field is invalid
        WS_NO_SEC_VERSION, // The WebSocket handshake Sec-WebSocket-Version field is missing
        WS_BAD_SEC_VERSION, // The WebSocket handshake Sec-WebSocket-Version field is invalid
        WS_NO_SEC_ACCEPT, // The WebSocket handshake Sec-WebSocket-Accept field is missing
        WS_BAD_SEC_ACCEPT, // The WebSocket handshake Sec-WebSocket-Accept field is invalid
        WS_BAD_OPCODE, // The WebSocket frame contained an illegal opcode
        WS_BAD_DATA_FRAME, // The WebSocket data frame was unexpected
        WS_BAD_CONTINUATION, // The WebSocket continuation frame was unexpected
        WS_BAD_RESERVED_BITS, // The WebSocket frame contained illegal reserved bits
        WS_BAD_CONTROL_FRAGMENT, // The WebSocket control frame was fragmented
        WS_BAD_CONTROL_SIZE, // The WebSocket control frame size was invalid
        WS_BAD_UNMASKED_FRAME, // The WebSocket frame was unmasked
        WS_BAD_MASKED_FRAME, // The WebSocket frame was masked
        WS_BAD_SIZE, // The WebSocket frame size was not canonical
        BAD_FRAME_PAYLOAD, // The WebSocket frame payload was not valid utf8
        WS_CLOSED, // The WebSocket receive close frame
        WS_COMPLETE_FRAME, // The WebSocket receive a complete frame
    };

    /// Error conditions corresponding to sets of error codes.
    enum class Condition {
        /** The WebSocket handshake failed

            This condition indicates that the WebSocket handshake failed. If
            the corresponding HTTP response indicates the keep-alive behavior,
            then the handshake may be reattempted.
        */
        WS_HANDSHAKE_FAILED = 1,

        /** A WebSocket protocol violation occurred

            This condition indicates that the remote peer on the WebSocket
            connection sent data which violated the protocol.
        */
        WS_PROTOCOL_VIOLATION
    };

    namespace Detail {
        class ErrorCode : public std::error_category {
        public:
            const char* name() const noexcept override
            {
                return "websocket";
            }

            std::string message(int ev) const override
            {
                switch (static_cast<Error>(ev)) {
                default:
                case Error::READ_MESSAGE_TOO_BIG:
                    return "The socket read message exceeded the locally configured limit";
                case Error::WRITE_MESSAGE_TOO_BIG:
                    return "The socket write message exceeded the locally configured limit";
                case Error::READ_TIMEOUT:
                    return "Socket read timeout";
                case Error::SEND_QUEUE_TOO_BIG:
                    return "The socket send message queue size exceeded configured(config.hpp) limit";
                case Error::INVALID_READ_OPERATION:
                    return "invalid read operation";
                case Error::WS_BAD_HTTP_HEADER:
                    return "Websocket parse http header failed";
                case Error::WS_BAD_HTTP_VERSION:
                    return "The WebSocket handshake was not HTTP/1.1";
                case Error::WS_BAD_METHOD:
                    return "The WebSocket handshake method was not GET";
                case Error::WS_BAD_HTTP_STATUS_CODE:
                    return "The WebSocket handshake http status code was not 101";
                case Error::WS_NO_HOST:
                    return "The WebSocket handshake Host field is missing";
                case Error::WS_NO_CONNECTION:
                    return "The WebSocket handshake Connection field is missing";
                case Error::WS_NO_CONNECTION_UPGRADE:
                    return "The WebSocket handshake Connection field is missing the upgrade token";
                case Error::WS_NO_UPGRADE:
                    return "The WebSocket handshake Upgrade field is missing";
                case Error::WS_NO_UPGRADE_WEBSOCKET:
                    return "The WebSocket handshake Upgrade field is missing the websocket token";
                case Error::WS_NO_SEC_KEY:
                    return "The WebSocket handshake Sec-WebSocket-Key field is missing";
                case Error::WS_BAD_SEC_KEY:
                    return "The WebSocket handshake Sec-WebSocket-Key field is invalid";
                case Error::WS_NO_SEC_VERSION:
                    return "The WebSocket handshake Sec-WebSocket-Version field is missing";
                case Error::WS_BAD_SEC_VERSION:
                    return "The WebSocket handshake Sec-WebSocket-Version field is invalid";
                case Error::WS_NO_SEC_ACCEPT:
                    return "The WebSocket handshake Sec-WebSocket-Accept field is missing";
                case Error::WS_BAD_SEC_ACCEPT:
                    return "The WebSocket handshake Sec-WebSocket-Accept field is invalid";

                case Error::WS_BAD_OPCODE:
                    return "The WebSocket frame contained an illegal opcode";
                case Error::WS_BAD_DATA_FRAME:
                    return "The WebSocket data frame was unexpected";
                case Error::WS_BAD_CONTINUATION:
                    return "The WebSocket continuation frame was unexpected";
                case Error::WS_BAD_RESERVED_BITS:
                    return "The WebSocket frame contained illegal reserved bits";
                case Error::WS_BAD_CONTROL_FRAGMENT:
                    return "The WebSocket control frame was fragmented";
                case Error::WS_BAD_CONTROL_SIZE:
                    return "The WebSocket control frame size was invalid";
                case Error::WS_BAD_UNMASKED_FRAME:
                    return "The WebSocket frame was unmasked";
                case Error::WS_BAD_MASKED_FRAME:
                    return "The WebSocket frame was masked";
                case Error::WS_BAD_SIZE:
                    return "The WebSocket frame size was not canonical";
                case Error::BAD_FRAME_PAYLOAD:
                    return "The WebSocket frame payload was not valid utf8";
                case Error::WS_CLOSED:
                    return "The WebSocket receive close frame";
                }
            }

            std::error_condition default_error_condition(int ev) const noexcept override
            {
                switch (static_cast<Error>(ev)) {
                default:
                case Error::BUFFER_OVERFLOW:
                case Error::READ_MESSAGE_TOO_BIG:
                case Error::WRITE_MESSAGE_TOO_BIG:
                case Error::READ_TIMEOUT:
                case Error::SEND_QUEUE_TOO_BIG:
                    return { ev, *this };
                case Error::WS_BAD_HTTP_VERSION:
                case Error::WS_BAD_METHOD:
                case Error::WS_NO_HOST:
                case Error::WS_NO_CONNECTION:
                case Error::WS_NO_CONNECTION_UPGRADE:
                case Error::WS_NO_UPGRADE:
                case Error::WS_NO_UPGRADE_WEBSOCKET:
                case Error::WS_NO_SEC_KEY:
                case Error::WS_BAD_SEC_KEY:
                case Error::WS_NO_SEC_VERSION:
                case Error::WS_BAD_SEC_VERSION:
                case Error::WS_NO_SEC_ACCEPT:
                case Error::WS_BAD_SEC_ACCEPT:
                    return Condition::WS_HANDSHAKE_FAILED;
                case Error::WS_BAD_OPCODE:
                case Error::WS_BAD_DATA_FRAME:
                case Error::WS_BAD_CONTINUATION:
                case Error::WS_BAD_RESERVED_BITS:
                case Error::WS_BAD_CONTROL_FRAGMENT:
                case Error::WS_BAD_CONTROL_SIZE:
                case Error::WS_BAD_UNMASKED_FRAME:
                case Error::WS_BAD_MASKED_FRAME:
                case Error::WS_BAD_SIZE:
                case Error::BAD_FRAME_PAYLOAD:
                    return Condition::WS_PROTOCOL_VIOLATION;
                }
            }
        };

        class error_conditions : public std::error_category {
        public:
            const char* name() const noexcept override
            {
                return "websocket";
            }

            std::string message(int cv) const override
            {
                switch (static_cast<Condition>(cv)) {
                default:
                case Condition::WS_HANDSHAKE_FAILED:
                    return "The WebSocket handshake failed";
                case Condition::WS_PROTOCOL_VIOLATION:
                    return "A WebSocket protocol violation occurred";
                }
            }
        };
    }

    inline std::error_code
    make_error_code(Error e)
    {
        static Detail::ErrorCode const cat {};
        return std::error_code { static_cast<
                                     std::underlying_type<Error>::type>(e),
            cat };
    }

    inline std::error_condition
    make_error_condition(Condition c)
    {
        static Detail::error_conditions const cat {};
        return std::error_condition { static_cast<
                                          std::underlying_type<Condition>::type>(c),
            cat };
    }
}
};

#endif // ERROR_HPP_