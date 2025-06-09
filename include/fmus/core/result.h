#pragma once

#include "error.h"
#include <utility>
#include <type_traits>
#include <cassert>

namespace fmus {
namespace core {

/**
 * @brief A class to represent the result of an operation that may fail
 *
 * @tparam T The type of the successful result value
 */
template <typename T>
class Result {
public:
    /**
     * @brief Construct a successful Result with a value
     *
     * @param value The successful result value
     */
    Result(const T& value) : m_hasValue(true), m_value(value), m_error(ErrorCode::Ok, "") {}

    /**
     * @brief Construct a successful Result with a value (move version)
     *
     * @param value The successful result value
     */
    Result(T&& value) : m_hasValue(true), m_value(std::move(value)), m_error(ErrorCode::Ok, "") {}

    /**
     * @brief Construct a failed Result with an error
     *
     * @param error The error that occurred
     */
    Result(const Error& error) : m_hasValue(false), m_error(error) {}

    /**
     * @brief Check if the result is successful
     *
     * @return true if the result is successful
     * @return false if an error occurred
     */
    bool isOk() const { return m_hasValue; }

    /**
     * @brief Check if the result is an error
     *
     * @return true if an error occurred
     * @return false if the result is successful
     */
    bool isError() const { return !m_hasValue; }

    /**
     * @brief Get the value (will assert if called on an error result)
     *
     * @return const T& The result value
     */
    const T& value() const {
        assert(m_hasValue && "Cannot access value of an error result");
        return m_value;
    }

    /**
     * @brief Get the value (will assert if called on an error result)
     *
     * @return T& The result value
     */
    T& value() {
        assert(m_hasValue && "Cannot access value of an error result");
        return m_value;
    }

    /**
     * @brief Get the error (will assert if called on a successful result)
     *
     * @return const Error& The error
     */
    const Error& error() const {
        assert(!m_hasValue && "Cannot access error of a successful result");
        return m_error;
    }

    /**
     * @brief Get the value if successful, or the provided default if an error
     *
     * @param defaultValue The default value to return on error
     * @return T The result value or the default value
     */
    T valueOr(const T& defaultValue) const {
        return m_hasValue ? m_value : defaultValue;
    }

    /**
     * @brief Execute a function if the result is successful
     *
     * @tparam Func The function type
     * @param f The function to execute with the value
     * @return Result<T> This result for chaining
     */
    template <typename Func>
    Result<T>& onSuccess(Func&& f) {
        if (m_hasValue) {
            f(m_value);
        }
        return *this;
    }

    /**
     * @brief Execute a function if the result is an error
     *
     * @tparam Func The function type
     * @param f The function to execute with the error
     * @return Result<T> This result for chaining
     */
    template <typename Func>
    Result<T>& onError(Func&& f) {
        if (!m_hasValue) {
            f(m_error);
        }
        return *this;
    }

private:
    bool m_hasValue;  ///< Whether the result contains a value
    T m_value;        ///< The value (valid if m_hasValue is true)
    Error m_error;    ///< The error (valid if m_hasValue is false)
};

/**
 * @brief Specialization of Result for void (no value)
 */
template <>
class Result<void> {
public:
    /**
     * @brief Construct a successful Result
     */
    Result() : m_isOk(true), m_error(ErrorCode::Ok, "") {}

    /**
     * @brief Construct a failed Result with an error
     *
     * @param error The error that occurred
     */
    Result(const Error& error) : m_isOk(false), m_error(error) {}

    /**
     * @brief Check if the result is successful
     *
     * @return true if the result is successful
     * @return false if an error occurred
     */
    bool isOk() const { return m_isOk; }

    /**
     * @brief Check if the result is an error
     *
     * @return true if an error occurred
     * @return false if the result is successful
     */
    bool isError() const { return !m_isOk; }

    /**
     * @brief Get the error (will assert if called on a successful result)
     *
     * @return const Error& The error
     */
    const Error& error() const {
        assert(!m_isOk && "Cannot access error of a successful result");
        return m_error;
    }

    /**
     * @brief Execute a function if the result is successful
     *
     * @tparam Func The function type
     * @param f The function to execute
     * @return Result<void> This result for chaining
     */
    template <typename Func>
    Result<void>& onSuccess(Func&& f) {
        if (m_isOk) {
            f();
        }
        return *this;
    }

    /**
     * @brief Execute a function if the result is an error
     *
     * @tparam Func The function type
     * @param f The function to execute with the error
     * @return Result<void> This result for chaining
     */
    template <typename Func>
    Result<void>& onError(Func&& f) {
        if (!m_isOk) {
            f(m_error);
        }
        return *this;
    }

private:
    bool m_isOk;    ///< Whether the result is successful
    Error m_error;  ///< The error (valid if m_isOk is false)
};

// Helper function to create a successful Result
template <typename T>
Result<T> makeOk(T&& value) {
    return Result<T>(std::forward<T>(value));
}

// Helper function to create a successful void Result
inline Result<void> makeOk() {
    return Result<void>();
}

// Helper function to create an error Result
template <typename T>
Result<T> makeError(ErrorCode code, const std::string& message) {
    return Result<T>(Error(code, message));
}

// Helper function to create an error void Result
inline Result<void> makeError(ErrorCode code, const std::string& message) {
    return Result<void>(Error(code, message));
}

} // namespace core
} // namespace fmus
