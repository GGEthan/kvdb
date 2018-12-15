


namespace kv_engine {

typedef int NodePointer;

template <class T>
class SerializableList {
public:
    struct ListNode {
        NodePointer next = -1;
        T data;
    };

    class Iterator {
    public:
        Iterator(ListNode* start, ListNode* ln) : _start(start), p(ln) {}
        ListNode* next();
    private:
        ListNode* p;
        ListNode* _start;
    };

    Iterator Begin();

    void Push(const T & data);
private:
    ListNode* _start = nullptr;
    NodePointer _head = -1;
    NodePointer _tail = -1;
    int size = 0;
};

template <class T>
SerializableList<T>::ListNode* SerializableList<T>::Iterator::next() {
    if (p->next == -1)
        return nullptr;
    p = _start + p->next;
    return p;
}

template <class T>
SerializableList<T>::Iterator SerializableList<T>::Begin() {
    return SerializableList<T>::Iterator(_start, _start + _head);
}


} // kv_engine