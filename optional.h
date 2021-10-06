#pragma once

#include <stdexcept>
#include <utility>
#include <new>

// »сключение этого типа должно генерироватс€ при обращении к пустому optional
class BadOptionalAccess : public std::exception
{
public:
    using exception::exception;

    virtual const char* what() const noexcept override
    {
        return "Bad optional access";
    }
};

template <typename T>
class Optional
{
public:
    Optional() = default;

    Optional(const T& value)
    {
        if (is_initialized_) // ≈сли пам€ть была инициализирована
        {
            *value_ = value;
        }
        else
        {
            value_ = new (&data_[0]) T(value);
            is_initialized_ = true;
        }
    }

    Optional(T&& value) noexcept
    {
        if (is_initialized_)
        {
            *value_ = std::move(value);
        }
        else
        {
            value_ = new (&data_[0]) T(std::move(value));
            is_initialized_ = true;
        }
    }

    Optional(const Optional& other)
    {
        if (other.is_initialized_ && is_initialized_)
        {
            *value_ = other.Value();
        }
        else if (other.is_initialized_ && !is_initialized_)
        {
            value_ = new (&data_[0]) T(other.Value());
            is_initialized_ = true;
        }
        else if (!other.is_initialized_ && is_initialized_)
        {
            Reset();
        }
    }

    Optional(Optional&& other) noexcept
    {
        if (other.is_initialized_ && is_initialized_)
        {
            *value_ = std::move(other.Value());
        }
        else if (other.is_initialized_ && !is_initialized_)
        {
            value_ = new (&data_[0]) T(std::move(other.Value()));
            is_initialized_ = true;
        }
    }

    ~Optional()
    {
        Reset();
    }

    Optional& operator=(const T& value)
    {
        if (is_initialized_) // ≈сли пам€ть была инициализирована
        {
            *value_ = value;
        }
        else
        {
            value_ = new (&data_[0]) T(value);
            is_initialized_ = true;
        }

        return *this;
    }

    Optional& operator=(T&& rhs)
    {
        if (is_initialized_)
        {
            *value_ = std::move(rhs);
        }
        else
        {
            value_ = new (&data_[0]) T(std::move(rhs));
            is_initialized_ = true;
        }
        return *this;
    }

    Optional& operator=(const Optional& rhs)
    {
        if (this == &rhs)
            return *this;

        if (rhs.is_initialized_ && is_initialized_)
        {
            *value_ = rhs.Value();
        }
        else if (rhs.is_initialized_ && !is_initialized_)
        {
            value_ = new (&data_[0]) T(rhs.Value());
            is_initialized_ = true;
        }
        else if (!rhs.is_initialized_ && is_initialized_)
        {
            Reset();
        }
        

        return *this;
    }

    Optional& operator=(Optional&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        if (rhs.is_initialized_ && is_initialized_)
        {
            *value_ = std::move(rhs.Value());
        }
        else if (rhs.is_initialized_ && !is_initialized_)
        {
            value_ = new (&data_[0]) T(std::move(rhs.Value()));
            is_initialized_ = true;
        }
        else if (!rhs.is_initialized_ && is_initialized_)
        {
            Reset();
        }

        return *this;
    }

    T* operator->()
    {
        return value_;
    }

    const T* operator->() const
    {
        return value_;
    }

    T& operator*() &
    {
        return *value_;
    }

    const T& operator*() const&
    {
        return *value_;
    }

    T&& operator*() &&
    {
        return std::move(*value_);
    }

    // ћетод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() &
    {
        if (!is_initialized_)
            throw BadOptionalAccess();
        else
            return *value_;
    }

    const T& Value() const&
    {
        if (!is_initialized_)
            throw BadOptionalAccess();
        else
            return *value_;
    }

    T&& Value() &&
    {
        if (!is_initialized_)
            throw BadOptionalAccess();
        else
            return std::move(*value_);
    }

    template< class... Args >
    T& Emplace(Args&&... args)
    {
        if (HasValue())
            Reset();

        value_ = new (&data_[0]) T(std::forward<Args>(args)...);
        is_initialized_ = true;
        return *value_;
    }

    void Reset()
    {
        if (is_initialized_)
        {
            value_->~T();
            is_initialized_ = false;
        }
    }

    bool HasValue() const
    {
        return is_initialized_;
    }

    void Swap(Optional& other)
    {
        std::swap(*value_, *other.value_);
        std::swap(is_initialized_, other.is_initialized_);
    }

private:
    // alignas нужен дл€ правильного выравнивани€ блока пам€ти
    alignas(T) char data_[sizeof(T)];
    T* value_ = nullptr;
    bool is_initialized_ = false;
};