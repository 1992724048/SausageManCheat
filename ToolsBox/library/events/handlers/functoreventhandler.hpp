#pragma once

#include <memory>
#include <assert.h>
#include "abstracteventhandler.hpp"
#include "helpers/innerholder.hpp"
#include "../helpers/is_equatable.hpp"


namespace events::handlers {
    template<class TFunctor, class... TParams>
    struct IsFunctorParamsCompatible {
    private:
        template<class TCheckedFunctor, class... TCheckedParams>
        static constexpr auto exists(decltype( std::declval<TCheckedFunctor>()(std::declval<TCheckedParams>()...) )* = nullptr) noexcept -> std::true_type;

        template<class TCheckedFunctor, class... TCheckedParams>
        static constexpr auto exists(...) noexcept -> std::false_type;

    public:
        static constexpr bool value = decltype( exists<TFunctor, TParams...>(nullptr) )::value;
    };


    template<class TFunctor>
    class FunctorHolder;

    template<class TFunctor, class... TParams>
    class FunctorEventHandler : public AbstractEventHandler<TParams...> {
        using MyType = FunctorEventHandler<TFunctor, TParams...>;
        using TFunctorHolderPtr = std::shared_ptr<FunctorHolder<TFunctor>>;

    public:
        FunctorEventHandler(TFunctorHolderPtr functorHolder) : AbstractEventHandler<TParams...>(), m_functorHolder(functorHolder) {
            assert(m_functorHolder != nullptr);
        }

        virtual auto call(TParams... params) -> void override {
            static_assert(IsFunctorParamsCompatible<TFunctor, TParams...>::value, "Event and functor arguments are not compatible");

            m_functorHolder->m_innerHolder.get()(params...);
        }

    protected:
        virtual auto isEquals(const AbstractEventHandler<TParams...>& other) const noexcept -> bool override {
            const MyType* _other = dynamic_cast<const MyType*>(&other);
            return (_other != nullptr && *m_functorHolder == *_other->m_functorHolder);
        }

    private:
        TFunctorHolderPtr m_functorHolder;
    };

    template<class TEqu, class TEnabled = void>
    struct EqualityChecker;

    template<class TEquatable>
    struct EqualityChecker<TEquatable, std::enable_if_t<is_equatable<TEquatable>::value>> {
        static constexpr auto isEquals(const TEquatable& operand1, const TEquatable& operand2) noexcept -> bool {
            return (operand1 == operand2);
        }
    };

    template<class TNonEquatable>
    struct EqualityChecker<TNonEquatable, std::enable_if_t<!is_equatable<TNonEquatable>::value>> {
        static constexpr auto isEquals(const TNonEquatable& operand1, const TNonEquatable& operand2) noexcept -> bool {
            return (&operand1 == &operand2);
        }
    };

    template<class TFunctor>
    class FunctorHolder {
        using MyType = FunctorHolder<TFunctor>;

    public:
        ~FunctorHolder() {
            delete &m_innerHolder;
        }

        template<class... TCallParams>
        operator TEventHandlerPtr<TCallParams...>() {
            return TEventHandlerPtr<TCallParams...>(new FunctorEventHandler<TFunctor, TCallParams...>(m_me.lock()));
        }

        auto operator==(const MyType& other) const noexcept -> bool {
            return EqualityChecker<TFunctor>::isEquals(m_innerHolder.get(), other.m_innerHolder.get());
        }

        auto operator!=(const MyType& other) const noexcept -> bool {
            return !(*this == other);
        }

        // TFunctor typename is reserved by the enclosing template so need something different
        template<class TArgFunctor>
        static auto create(TArgFunctor&& functor) -> std::shared_ptr<MyType> {
            std::shared_ptr<MyType> result(new MyType(std::forward<TArgFunctor>(functor)));
            result->m_me = result;
            return result;
        }

    private:
        template<class TArgFunctor>
        FunctorHolder(TArgFunctor&& functor) : m_innerHolder(createInnerHolder<TFunctor>(std::forward<TArgFunctor>(functor))), m_me() {}

        AbstractInnerHolder<TFunctor>& m_innerHolder;

        std::weak_ptr<MyType> m_me;

        template<class TArgFunctor, class...>
        friend class FunctorEventHandler;
    };


    template<class TFunctor>
    auto createFunctorEventHandler(TFunctor&& functor) -> std::shared_ptr<FunctorHolder<std::decay_t<TFunctor>>> {
        return FunctorHolder<std::decay_t<TFunctor>>::create(std::forward<TFunctor>(functor));
    }
}


#define     FUNCTOR_HANDLER( Functor )              ::events::handlers::createFunctorEventHandler( Functor )
#define     LAMBDA_HANDLER( Lambda )                FUNCTOR_HANDLER( Lambda )
#define     STD_FUNCTION_HANDLER( StdFunction )     FUNCTOR_HANDLER( StdFunction )
#define     FUNCTION_HANDLER( Function )            FUNCTOR_HANDLER( &Function )
