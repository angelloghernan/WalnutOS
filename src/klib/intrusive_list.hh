#pragma once
#include "option.hh"

namespace wlib {
    // A generic intrusive list. As the structure implies, all elements in the list 
    // must live the entire time they are contained within the intrusive list.
    // In addition, any type T which is used for the intrusive list must have the
    // list_links struct included in it.
    template<typename T>
    class IntrusiveList {
    public:
        constexpr IntrusiveList() : head(Option<T&>()), tail(Option<T&>()) {}

        // Push a single element at the head of the list.
        // Will always succeed without any undefined behavior so long as the element is valid.
        void constexpr push_front(T& element) {
            auto old_head = head;
            head = element;

            head.unwrap().links.next = old_head;
            head.unwrap().links.prev = Option<T&>();
            
            if (!tail) [[unlikely]] {
                tail = element;
            }
        }

        // Push a single element at the head of the list.
        // Will always succeed without any undefined behavior so long as the element is valid.
        void constexpr push_back(T& element) {
            auto old_tail = tail;
            tail = element;

            tail.unwrap().links.next = Option<T&>();
            tail.unwrap().links.prev = old_tail;

            if (!head) [[unlikely]] {
                head = element;
            }
        }

        // Pop from the front of the list.
        // Return None if there are no elements.
        auto constexpr pop_front() -> Option<T> {
            if (!head) [[unlikely]] {
                return {};
            }

            auto const& old_head = head.unwrap();
            head = old_head.links.next;
            if (tail == old_head) [[unlikely]] {
                tail = Option<T&>();
            }
            return old_head;
        }

        // Pop from the back of the list.
        // Return None if there are no elements.
        auto constexpr pop_back() -> Option<T> {
            if (!tail) [[unlikely]] {
                return {};
            }

            auto const& old_tail = tail.unwrap();
            tail = old_tail.links.prev;
            if (head == old_tail) [[unlikely]] {
                head = Option<T&>();
            }
            return old_tail;
        }

    private:
        Option<T&> head;
        Option<T&> tail;
    };


    // list_links: For use in classes/structs using IntrusiveList. Place as a public
    // member and name it "links". Occupies 8 bytes on x86 (two pointers).
    template<typename T>
    struct list_links {
        Option<T&> next = {};
        Option<T&> prev = {};
    };
};
