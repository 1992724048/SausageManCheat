#pragma once

#include <memory>
#include <assert.h>
#include "abstracteventhandler.hpp"
#include "helpers/innerholder.hpp"


namespace events::handlers {
    template<class TMethodHolder, class... TParams>
    struct IsMethodParamsCompatible {
    private:
        template<class TCheckedMethodHolder, class... TCheckedParams>
        static constexpr auto exists(
                decltype( (std::declval<TCheckedMethodHolder>().m_innerHolder.get().*std::declval<TCheckedMethodHolder>().m_method)(std::declval<TCheckedParams>()...) )* = nullptr) noexcept ->
            std::true_type;

        template<class TCheckedMethodHolder, class... TCheckedParams>
        static constexpr auto exists(...) noexcept -> std::false_type;

    public:
        static constexpr bool value = decltype( exists<TMethodHolder, TParams...>(nullptr) )::value;
    };


    template<class TMethodHolder, class... TParams>
    class MethodEventHandler : public AbstractEventHandler<TParams...> {
        using MyType = MethodEventHandler<TMethodHolder, TParams...>;
        using TMethodHolderPtr = std::shared_ptr<TMethodHolder>;

    public:
        MethodEventHandler(TMethodHolderPtr methodHolder) : AbstractEventHandler<TParams...>(), m_methodHolder(methodHolder) {
            assert(m_methodHolder != nullptr);
        }

        virtual auto call(TParams... params) -> void override {
            static_assert(IsMethodParamsCompatible<TMethodHolder, TParams...>::value, "Event and method arguments are not compatible");

            (m_methodHolder->m_innerHolder.get().*m_methodHolder->m_method)(params...);
        }

    protected:
        virtual auto isEquals(const AbstractEventHandler<TParams...>& other) const noexcept -> bool override {
            const MyType* _other = dynamic_cast<const MyType*>(&other);
            return (_other != nullptr && *m_methodHolder == *_other->m_methodHolder);
        }

    private:
        TMethodHolderPtr m_methodHolder;
    };


    template<class TObject, class TResult, class... TParams>
    class MethodHolder {
        using MyType = MethodHolder<TObject, TResult, TParams...>;
        using TMethod = TResult(TObject::*)(TParams...);

    public:
        ~MethodHolder() {
            delete &m_innerHolder;
        }

        template<class... TCallParams>
        operator TEventHandlerPtr<TCallParams...>() {
            return TEventHandlerPtr<TCallParams...>(new MethodEventHandler<MyType, TCallParams...>(m_me.lock()));
        }

        auto operator==(const MyType& other) const noexcept -> bool {
            return (&m_innerHolder.get() == &other.m_innerHolder.get() && m_method == other.m_method);
        }

        auto operator!=(const MyType& other) const noexcept -> bool {
            return !(*this == other);
        }

        // TObject typename is reserved by the enclosing template so need something different
        template<class TArgObject>
        static auto create(TArgObject&& object, TMethod method) -> std::shared_ptr<MyType> {
            std::shared_ptr<MyType> result(new MyType(std::forward<TArgObject>(object), method));
            result->m_me = result;
            return result;
        }

    private:
        template<class TArgObject>
        MethodHolder(TArgObject&& object, TMethod method) : m_innerHolder(createInnerHolder<TObject>(std::forward<TArgObject>(object))), m_method(method) {
            assert(m_method != nullptr);
        }

        AbstractInnerHolder<TObject>& m_innerHolder;
        TMethod m_method;

        std::weak_ptr<MyType> m_me;

        template<class TMethodHolder, class...>
        friend class MethodEventHandler;
        template<class TMethodHolder, class...>
        friend struct IsMethodParamsCompatible;
    };


    template<class TObject, class TResult, class... TParams>
    auto createMethodEventHandler(TObject&& object, TResult (std::decay_t<TObject>::*method)(TParams...)) -> std::shared_ptr<MethodHolder<std::decay_t<TObject>, TResult, TParams...>> {
        return MethodHolder<std::decay_t<TObject>, TResult, TParams...>::create(std::forward<TObject>(object), method);
    }
}


#define     METHOD_HANDLER( Object, Method )     ::events::handlers::createMethodEventHandler( Object, &Method )
#define     MY_METHOD_HANDLER( Method )          METHOD_HANDLER( *this, Method )
