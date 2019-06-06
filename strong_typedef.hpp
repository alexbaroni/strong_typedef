#ifndef JSS_STRONG_TYPEDEF_HPP
#define JSS_STRONG_TYPEDEF_HPP
#include <type_traits>
#include <utility>
#include <functional>
#include <ostream>

namespace jss {
    namespace detail {
        template <typename Derived, typename ValueType, typename... Properties>
        class strong_typedef_base
            : public Properties::template mixin<Derived, ValueType>... {
        public:
            using underlying_value_type= ValueType;

            constexpr strong_typedef_base() noexcept : value() {}

            explicit constexpr strong_typedef_base(ValueType value_) noexcept(
                std::is_nothrow_move_constructible<ValueType>::value) :
                value(std::move(value_)) {}

            explicit constexpr operator ValueType const &() const noexcept {
                return value;
            }

            constexpr ValueType const &underlying_value() const noexcept {
                return value;
            }

            constexpr ValueType &underlying_value() noexcept {
                return value;
            }

        private:
            ValueType value;
        };

    }

    template <typename Tag, typename ValueType, typename... Properties>
    struct strong_typedef : detail::strong_typedef_base<
                                strong_typedef<Tag, ValueType, Properties...>,
                                ValueType, Properties...> {
        using detail::strong_typedef_base<
            strong_typedef<Tag, ValueType, Properties...>, ValueType,
            Properties...>::strong_typedef_base;
    };

    namespace strong_typedef_properties {
        struct equality_comparable {
            template <typename Derived, typename ValueType> struct mixin {
                friend bool
                operator==(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() ==
                        std::declval<ValueType const &>())) {
                    return lhs.underlying_value() == rhs.underlying_value();
                }
                friend bool
                operator!=(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() !=
                        std::declval<ValueType const &>())) {
                    return lhs.underlying_value() != rhs.underlying_value();
                }
            };
        };

        struct pre_incrementable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived &operator++(Derived &self) noexcept(
                    noexcept(++std::declval<ValueType &>())) {
                    ++self.underlying_value();
                    return self;
                }
            };
        };

        struct post_incrementable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived operator++(
                    Derived &self,
                    int) noexcept(noexcept(std::declval<ValueType &>()++)) {
                    return Derived{self.underlying_value()++};
                }
            };
        };

        struct incrementable {
            template <typename Derived, typename ValueType>
            struct mixin : pre_incrementable::mixin<Derived, ValueType>,
                           post_incrementable::mixin<Derived, ValueType> {};
        };

        struct pre_decrementable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived &operator--(Derived &self) noexcept(
                    noexcept(--std::declval<ValueType &>())) {
                    --self.underlying_value();
                    return self;
                }
            };
        };

        struct post_decrementable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived operator--(
                    Derived &self,
                    int) noexcept(noexcept(std::declval<ValueType &>()--)) {
                    return Derived{self.underlying_value()--};
                }
            };
        };

        struct decrementable {
            template <typename Derived, typename ValueType>
            struct mixin : pre_decrementable::mixin<Derived, ValueType>,
                           post_decrementable::mixin<Derived, ValueType> {};
        };

        struct generic_mixed_addable {
            template <typename Derived, typename ValueType> struct mixin {
                template <typename Rhs>
                friend constexpr typename std::enable_if<
                    !std::is_same<Rhs, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() +
                                std::declval<Rhs const &>()),
                            ValueType>::value,
                    Derived>::type
                operator+(Derived const &lhs, Rhs const &rhs) noexcept(noexcept(
                    std::declval<ValueType const &>() +
                    std::declval<Rhs const &>())) {
                    return Derived{lhs.underlying_value() + rhs};
                }

                template <typename Lhs>
                friend constexpr typename std::enable_if<
                    !std::is_same<Lhs, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Lhs const &>() +
                                std::declval<ValueType const &>()),
                            ValueType>::value,
                    Derived>::type
                operator+(Lhs const &lhs, Derived const &rhs) noexcept(noexcept(
                    std::declval<Lhs const &>() +
                    std::declval<ValueType const &>())) {
                    return Derived{lhs + rhs.underlying_value()};
                }
            };
        };

        template <typename Other> struct mixed_addable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() +
                                std::declval<Other const &>()),
                            ValueType>::value,
                    Derived>::type
                operator+(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() +
                        std::declval<Other const &>())) {
                    return Derived{lhs.underlying_value() + rhs};
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() +
                                std::declval<ValueType const &>()),
                            ValueType>::value,
                    Derived>::type
                operator+(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() +
                        std::declval<ValueType const &>())) {
                    return Derived{lhs + rhs.underlying_value()};
                }
            };
        };

        struct self_addable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived
                operator+(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() +
                        std::declval<ValueType const &>())) {
                    return Derived{lhs.underlying_value() +
                                   rhs.underlying_value()};
                }
            };
        };

        struct addable {
            template <typename Derived, typename ValueType>
            struct mixin
                : self_addable::mixin<Derived, ValueType>,
                  mixed_addable<ValueType>::template mixin<Derived, ValueType> {
            };
        };

        struct generic_mixed_subtractable {
            template <typename Derived, typename ValueType> struct mixin {
                template <typename Rhs>
                friend constexpr typename std::enable_if<
                    !std::is_same<Rhs, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() -
                                std::declval<Rhs const &>()),
                            ValueType>::value,
                    Derived>::type
                operator-(Derived const &lhs, Rhs const &rhs) noexcept(noexcept(
                    std::declval<ValueType const &>() -
                    std::declval<Rhs const &>())) {
                    return Derived{lhs.underlying_value() - rhs};
                }

                template <typename Lhs>
                friend constexpr typename std::enable_if<
                    !std::is_same<Lhs, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Lhs const &>() -
                                std::declval<ValueType const &>()),
                            ValueType>::value,
                    Derived>::type
                operator-(Lhs const &lhs, Derived const &rhs) noexcept(noexcept(
                    std::declval<Lhs const &>() -
                    std::declval<ValueType const &>())) {
                    return Derived{lhs - rhs.underlying_value()};
                }
            };
        };

        template <typename Other> struct mixed_subtractable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() -
                                std::declval<Other const &>()),
                            ValueType>::value,
                    Derived>::type
                operator-(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() -
                        std::declval<Other const &>())) {
                    return Derived{lhs.underlying_value() - rhs};
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() -
                                std::declval<ValueType const &>()),
                            ValueType>::value,
                    Derived>::type
                operator-(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() -
                        std::declval<ValueType const &>())) {
                    return Derived{lhs - rhs.underlying_value()};
                }
            };
        };

        struct self_subtractable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived
                operator-(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() -
                        std::declval<ValueType const &>())) {
                    return Derived{lhs.underlying_value() -
                                   rhs.underlying_value()};
                }
            };
        };

        struct subtractable {
            template <typename Derived, typename ValueType>
            struct mixin : self_subtractable::mixin<Derived, ValueType>,
                           mixed_subtractable<ValueType>::template mixin<
                               Derived, ValueType> {};
        };

        template <typename DifferenceType> struct difference {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr DifferenceType
                operator-(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() -
                        std::declval<ValueType const &>())) {
                    return DifferenceType{lhs.underlying_value() -
                                          rhs.underlying_value()};
                }
            };
        };

        struct ordered {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr bool
                operator<(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() <
                        std::declval<ValueType const &>())) {
                    return lhs.underlying_value() < rhs.underlying_value();
                }
                friend constexpr bool
                operator>(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() >
                        std::declval<ValueType const &>())) {
                    return lhs.underlying_value() > rhs.underlying_value();
                }
                friend constexpr bool
                operator<=(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() <=
                        std::declval<ValueType const &>())) {
                    return lhs.underlying_value() <= rhs.underlying_value();
                }
                friend constexpr bool
                operator>=(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() >=
                        std::declval<ValueType const &>())) {
                    return lhs.underlying_value() >= rhs.underlying_value();
                }
            };
        };

        template <typename Other> struct mixed_ordered {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() <
                                std::declval<Other const &>()),
                            bool>::value,
                    bool>::type
                operator<(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() <
                        std::declval<Other const &>())) {
                    return lhs.underlying_value() < rhs;
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() <
                                std::declval<ValueType const &>()),
                            bool>::value,
                    bool>::type
                operator<(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() <
                        std::declval<ValueType const &>())) {
                    return lhs < rhs.underlying_value();
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() >
                                std::declval<Other const &>()),
                            bool>::value,
                    bool>::type
                operator>(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() >
                        std::declval<Other const &>())) {
                    return lhs.underlying_value() > rhs;
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() >
                                std::declval<ValueType const &>()),
                            bool>::value,
                    bool>::type
                operator>(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() >
                        std::declval<ValueType const &>())) {
                    return lhs > rhs.underlying_value();
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() >=
                                std::declval<Other const &>()),
                            bool>::value,
                    bool>::type
                operator>=(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() >=
                        std::declval<Other const &>())) {
                    return lhs.underlying_value() >= rhs;
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() >=
                                std::declval<ValueType const &>()),
                            bool>::value,
                    bool>::type
                operator>=(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() >=
                        std::declval<ValueType const &>())) {
                    return lhs >= rhs.underlying_value();
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() <=
                                std::declval<Other const &>()),
                            bool>::value,
                    bool>::type
                operator<=(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() <=
                        std::declval<Other const &>())) {
                    return lhs.underlying_value() <= rhs;
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() <=
                                std::declval<ValueType const &>()),
                            bool>::value,
                    bool>::type
                operator<=(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() <=
                        std::declval<ValueType const &>())) {
                    return lhs <= rhs.underlying_value();
                }
            };
        };

        struct hashable {
            struct base {};
            template <typename Derived, typename ValueType>
            struct mixin : base {};
        };

        struct streamable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr std::ostream &
                operator<<(std::ostream &os, Derived const &st) {
                    return os << st.underlying_value();
                }
            };
        };

        struct comparable {
            template <typename Derived, typename ValueType>
            struct mixin
                : ordered::template mixin<Derived, ValueType>,
                  equality_comparable::template mixin<Derived, ValueType> {};
        };

        struct self_multiplicable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived operator
                    *(Derived const &lhs, Derived const &rhs) noexcept(noexcept(
                        std::declval<ValueType const &>() *
                        std::declval<ValueType const &>())) {
                    return Derived{lhs.underlying_value() *
                                   rhs.underlying_value()};
                }
            };
        };

        template <typename Other> struct mixed_multiplicable {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() *
                                std::declval<Other const &>()),
                            ValueType>::value,
                    Derived>::type
                operator*(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() *
                        std::declval<Other const &>())) {
                    return Derived{lhs.underlying_value() * rhs};
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() *
                                std::declval<ValueType const &>()),
                            ValueType>::value,
                    Derived>::type
                operator*(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() *
                        std::declval<ValueType const &>())) {
                    return Derived{lhs * rhs.underlying_value()};
                }
            };
        };

        struct multiplicable {
            template <typename Derived, typename ValueType>
            struct mixin
                : self_multiplicable::template mixin<Derived, ValueType>,
                  mixed_multiplicable<ValueType>::template mixin<
                      Derived, ValueType> {};
        };

        struct self_divisible {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr Derived
                operator/(Derived const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() /
                        std::declval<ValueType const &>())) {
                    return Derived{lhs.underlying_value() /
                                   rhs.underlying_value()};
                }
            };
        };

        template <typename Other> struct mixed_divisible {
            template <typename Derived, typename ValueType> struct mixin {
                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<ValueType const &>() /
                                std::declval<Other const &>()),
                            ValueType>::value,
                    Derived>::type
                operator/(Derived const &lhs, Other const &rhs) noexcept(
                    noexcept(
                        std::declval<ValueType const &>() /
                        std::declval<Other const &>())) {
                    return Derived{lhs.underlying_value() / rhs};
                }

                friend constexpr typename std::enable_if<
                    !std::is_same<Other, Derived>::value &&
                        std::is_convertible<
                            decltype(
                                std::declval<Other const &>() /
                                std::declval<ValueType const &>()),
                            ValueType>::value,
                    Derived>::type
                operator/(Other const &lhs, Derived const &rhs) noexcept(
                    noexcept(
                        std::declval<Other const &>() /
                        std::declval<ValueType const &>())) {
                    return Derived{lhs / rhs.underlying_value()};
                }
            };
        };

        struct divisible {
            template <typename Derived, typename ValueType>
            struct mixin : self_divisible::template mixin<Derived, ValueType>,
                           mixed_divisible<ValueType>::template mixin<
                               Derived, ValueType> {};
        };

    }
}

namespace std {
    template <typename Tag, typename ValueType, typename... Properties>
    struct hash<jss::strong_typedef<Tag, ValueType, Properties...>> {
        template <typename Arg>
        typename std::enable_if<
            std::is_same<
                Arg,
                jss::strong_typedef<Tag, ValueType, Properties...>>::value &&
                std::is_base_of<
                    jss::strong_typedef_properties::hashable::base, Arg>::value,
            size_t>::type
        operator()(Arg const &arg) const noexcept(noexcept(
            std::hash<ValueType>()(std::declval<ValueType const &>()))) {
            return std::hash<ValueType>()(arg.underlying_value());
        }
    };

}

#endif
