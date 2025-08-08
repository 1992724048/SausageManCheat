#pragma once

#include "../events/event.hpp"
#include "FieldEntry.h"
#include "FieldSerialize.h"

namespace config::internal {
    template<typename T>
    class FieldBase {
        using _FieldBaseT = FieldBase;

    public:
        using _ValueType = T;

        FieldBase() : field_changed_event(m_field_changed_event), p_container(nullptr) {}

        FieldBase(FieldSerialize<T>* _serialize_field_ptr) : field_changed_event(m_field_changed_event), p_container(_serialize_field_ptr) {
            p_container->changed_event += MY_METHOD_HANDLER(_FieldBaseT::on_field_changed);
        }

        FieldBase(const std::shared_ptr<FieldSerialize<T>>& _serialize_field) : field_changed_event(m_field_changed_event), p_container(_serialize_field) {
            p_container->changed_event += MY_METHOD_HANDLER(_FieldBaseT::on_field_changed);
        }

        FieldBase(const std::string& _friendly_name, const std::string& _name, const std::string& _section, T _default_value, bool _multi_profile = false) : field_changed_event(m_field_changed_event),
            p_container(std::make_shared<FieldSerialize<T>>(_friendly_name, _name, _section, _default_value, _multi_profile)) {
            p_container->changed_event += MY_METHOD_HANDLER(_FieldBaseT::on_field_changed);
        }

        FieldBase(const FieldBase& _field) : field_changed_event(m_field_changed_event), m_field_changed_event(), p_container(_field.p_container) {
            p_container->changed_event += MY_METHOD_HANDLER(FieldBase::on_field_changed);
        }

        ~FieldBase() {
            if (p_container.get() != nullptr) {
                p_container->changed_event -= MY_METHOD_HANDLER(FieldBase::on_field_changed);
            }
        }

        auto name() const -> std::string {
            return p_container->get_name();
        }

        auto friend_name() const -> std::string {
            return p_container->get_friend_name();
        }

        auto section() const -> std::string {
            return p_container->get_section();
        }

        auto shared() const -> bool {
            return p_container->is_shared();
        }

        auto value() const -> T& {
            std::shared_lock lock(mutex);
            return p_container->m_value;
        }

        auto pointer() const -> T* {
            std::shared_lock lock(mutex);
            return &p_container->m_value;
        }

        auto entry() const -> std::shared_ptr<FieldEntry> {
            return std::static_pointer_cast<FieldEntry>(p_container);
        }

        operator T&() const {
            return value();
        }

        operator T*() const {
            return pointer();
        }

        auto fire_changed() const -> void {
            p_container->fire_changed();
        }

        auto repos(const std::string& _new_section, bool _shared = false) -> void {
            p_container->reposition(_new_section, _shared);
        }

        auto move(const std::string& _new_section, bool _shared = false) -> void {
            p_container->move(_new_section, _shared);
        }

        auto operator=(const T& _other) -> FieldBase& {
            {
                std::unique_lock lock(mutex);
                p_container->m_value = _other;
            }
            p_container->fire_changed();
            return *this;
        }

        auto operator=(T&& _other) -> FieldBase& {
            {
                std::unique_lock lock(mutex);
                p_container->m_value = std::move(_other);
            }
            p_container->fire_changed();
            return *this;
        }

        auto operator=(std::shared_ptr<FieldSerialize<T>>& _other) -> FieldBase& {
            std::unique_lock lock(mutex);
            p_container->changed_event -= MY_METHOD_HANDLER(FieldBase::on_field_changed);

            p_container = _other;
            p_container->changed_event += MY_METHOD_HANDLER(FieldBase::on_field_changed);
            return *this;
        }

        auto operator=(FieldSerialize<T>* _other) -> FieldBase& {
            std::unique_lock lock(mutex);
            p_container->changed_event -= MY_METHOD_HANDLER(FieldBase::on_field_changed);

            p_container = std::make_shared<FieldSerialize<T>>(_other);
            p_container->changed_event += MY_METHOD_HANDLER(FieldBase::on_field_changed);
            return *this;
        }

        auto operator=(const FieldBase& _other) -> FieldBase& {
            std::unique_lock lock(mutex);
            p_container = _other.p_container;
            p_container->changed_event += MY_METHOD_HANDLER(FieldBase::on_field_changed);
            return *this;
        }

        IEvent<T&>& field_changed_event;

    protected:
        TEvent<T&> m_field_changed_event;

        std::shared_ptr<FieldSerialize<T>> p_container;
        mutable std::shared_mutex mutex;

        auto on_field_changed(FieldEntry* _entry) -> void {
            m_field_changed_event(value());
        }
    };
}
