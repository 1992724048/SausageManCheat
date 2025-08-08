#pragma once

#include "abstracteventjoin.h"
#include "../handlers/eventhandlerptr.h"


template <class... TParams>
class IEvent;

namespace events::joins {
	template <class... TParams>
	class HandlerEventJoin : public AbstractEventJoin {
	public:
		HandlerEventJoin(IEvent<TParams...>& _event, handlers::TEventHandlerPtr<TParams...> handler) : AbstractEventJoin(), m_event(_event), m_handler(handler) {
		}

		virtual inline auto isJoined() const -> bool override;
		virtual inline auto join() -> bool override;
		virtual inline auto unjoin() -> bool override;

	private:
		IEvent<TParams...>& m_event;
		handlers::TEventHandlerPtr<TParams...> m_handler;
	};
}
