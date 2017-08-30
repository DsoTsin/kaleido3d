#pragma once

namespace k3d
{
template <typename T>
class LockFreeQueue {
public:

	LockFreeQueue() : m_Size(0), m_Head(new Node()), m_Tail(m_Head) { }

	~LockFreeQueue() {
		while (DequeueAndDelete());
		delete m_Head;
	}

	bool IsEmpty() const { return m_Head->m_Next == nullptr; }

	void Enqueue(const T& v) {
		Node* node = new Node(v);
		while (1) 
        {
			Node* last = m_Tail;
			Node* next = last->m_Next;
			if (last != m_Tail) 
            { 
                continue; 
            }
			if (next == nullptr) 
            {
				if (__intrinsics__::AtomicCASPointer((void**)&last->m_Next, node, next))
                {
					__intrinsics__::AtomicCASPointer((void**)&m_Tail, node, last);
					++m_Size;
					return;
				}
			}
			else 
            {
                __intrinsics__::AtomicCASPointer((void**)&m_Tail, next, last);
			}
		}
	}

	bool Dequeue(T& Val) { // it returns false if there is nothing to deque.
		while (1) 
        {
			Node* first = m_Head;
			Node* last = m_Tail;
			Node* next = first->m_Next;

			if (first != m_Head) { continue; }

			if (first == last) 
            {
				if (next == nullptr)
                {
					return false;
				}
				__intrinsics__::AtomicCASPointer((void**)&m_Tail, next, last);
			}
			else {
				T result = next->m_Value;
				if (__intrinsics__::AtomicCASPointer((void**)&m_Head, next, first)) 
                {
					delete first;
                    Val = result;
					return true;
				}
			}
		}
        return false;
	}

	bool DequeueAndDelete() { // it destroys dequed item. but fast.
		while (1) {
			Node* first = m_Head;
			Node* last = m_Tail;
			Node* next = first->m_Next;

			if (first != m_Head) { continue; }

			if (first == last) {
				if (next == nullptr) {
					return false;
				}
				__intrinsics__::AtomicCASPointer((void**)&m_Tail, next, last);
			}
			else {
				if (__intrinsics__::AtomicCASPointer((void**)&m_Head, next, first)) {
					delete first;
					return true;
				}
			}
		}
	}


	LockFreeQueue(const LockFreeQueue&&) = delete;
	LockFreeQueue(const LockFreeQueue&) = delete;
	LockFreeQueue & operator=(const LockFreeQueue&) = delete;

private:

	class Node {
	public:
		const T m_Value;
		Node* m_Next;
		Node(const T& v) : m_Value(v), m_Next(nullptr) {}
		Node() : m_Value(), m_Next(nullptr) {};
	private:
		Node(const Node&) = delete;
		Node& operator=(const Node&) = delete;
	};

	volatile int m_Size;

	Node *m_Head, *m_Tail;
};

}