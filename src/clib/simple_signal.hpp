#ifndef IOTSMARTGATEWAY_SIMPLE_SIGNAL_H
#define IOTSMARTGATEWAY_SIMPLE_SIGNAL_H

#include <functional>
#include <string>

namespace Simple {
    constexpr const size_t default_max_callbacks = 1000;

    template<class R, class... Args> class Signal;

    /// Signal template specialised for the callback signature
    template<class R, class... Args>
    class Signal<R (Args...)> {
    protected:
        using CbFunction = std::function<R (Args...)>;
    private:
        std::pair<void*,void*> margins_;
        void ** free_beg_;
        size_t size_;

        [[nodiscard]] inline bool is_inside_buff(const void * cb_ptr) const {
            return cb_ptr >= margins_.first && cb_ptr < margins_.second;
        }

    public:
        class SlotHandle {
            const void ** handle_;
            explicit SlotHandle (void ** handle)
                : handle_(const_cast<const void **>(handle)) {
            };
            [[nodiscard]] void ** get_handle () const { return const_cast<void**>(handle_); };
        public:
            SlotHandle ()                       = default;
            SlotHandle (SlotHandle &&) noexcept = default;
            SlotHandle (const SlotHandle&)      = default;
            SlotHandle& operator= (SlotHandle&&) noexcept = default;
            SlotHandle& operator= (const SlotHandle&)     = default;
            friend Signal<R (Args...)>;
        };

        class SlotInvalid : std::exception {
            std::string e;
            explicit SlotInvalid (std::string reason)
                : e(std::move(reason)) {
            };
        public:
            SlotInvalid ()                   = delete;
            SlotInvalid (SlotInvalid&&)      = delete;
            SlotInvalid (const SlotInvalid&) = delete;
            [[nodiscard]] const char * what() const noexcept override {
                return e.c_str();
            }
            friend Signal<R (Args...)>;
        };

        /// Signal constructor, connects default callback if given.
        explicit Signal (const size_t &upper_limit = default_max_callbacks, CbFunction && method = CbFunction())
            : margins_(nullptr, nullptr), free_beg_(new void*[upper_limit+1]), size_(0) {
            margins_.first = free_beg_;
            for (size_t i = 0; i < upper_limit; ++i)
                free_beg_[i] = static_cast<void*>(&free_beg_[i + 1]);
            free_beg_[upper_limit] = nullptr;
            margins_.second = free_beg_ + upper_limit + 1;
            if (method) static_cast<void>(connect(std::move(method)));
        }
        Signal (Signal&&) noexcept            = default;
        Signal& operator= (Signal&&) noexcept = default;

        Signal (const Signal&)            = delete;
        Signal& operator= (const Signal&) = delete;

        /// Signal destructor releases all resources associated with this signal.
        ~Signal () {
            size_t left = size_;
            void ** cb_ptr = static_cast<void**>(margins_.first);
            while (left && cb_ptr < static_cast<void**>(margins_.second)) {
                if (*cb_ptr && !is_inside_buff(*cb_ptr)) {
                    delete reinterpret_cast<CbFunction*>(*cb_ptr); --left;
                }
                ++cb_ptr;
            }
            delete [] static_cast<void**>(margins_.first);
        };

        /// Operator to add a new slot, returns a handler.
        [[nodiscard]] SlotHandle connect (CbFunction && cbf) {
            if (!free_beg_ || !*free_beg_) throw std::exception();// SlotInvalid("Run out of slots");
            void** cbf_ptr = std::exchange(free_beg_, static_cast<void**>(*free_beg_));
            *cbf_ptr = new CbFunction(cbf); ++size_;
            return SlotHandle(cbf_ptr);
        }

        /// This function connects an std::function binding to the object member function pointer
        template<class Instance, class Class>
        [[nodiscard]] SlotHandle connect_slot (Instance &object, R (Class::*method) (Args...)) {
            return connect([&object, method] (Args... args) { return (object .* method) (args...); });
        }

        /// This function connects an std::function binding to the object member function pointer
        template<class Class>
        [[nodiscard]] SlotHandle connect_slot (Class *object, R (Class::*method) (Args...)) {
            return connect([object, method] (Args... args) { return (object ->* method) (args...); });
        }

        /// Operator to removes a slot through its handle, throws if no such slot.
        void disconnect (const SlotHandle & slot_handle) {
            void ** cbf_ptr = slot_handle.get_handle();
            if (!cbf_ptr  || !is_inside_buff(cbf_ptr) || !*cbf_ptr || is_inside_buff(*cbf_ptr))
                throw std::exception();// SlotInvalid("No such slot");
            delete reinterpret_cast<CbFunction*>(*cbf_ptr);
            *cbf_ptr = std::exchange(*free_beg_, cbf_ptr); --size_;
        }

        /// Emit a signal, i.e. invoke all its callbacks.
        void emit (Args&& ...args) const {
            size_t left = size_;
            void ** cb_ptr = static_cast<void**>(margins_.first);
            while (left && cb_ptr < static_cast<void**>(margins_.second)) {
                if (*cb_ptr && !is_inside_buff(*cb_ptr)) {
                    reinterpret_cast<CbFunction*>(*cb_ptr)->operator()(std::forward<Args>(args)...);
                    --left;
                }
                ++cb_ptr;
            }
        }

        [[nodiscard]] std::size_t size () const {
            return size_;
        }
    };
}

#endif //IOTSMARTGATEWAY_SIMPLE_SIGNAL_H
