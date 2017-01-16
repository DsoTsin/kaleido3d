#pragma once

K3D_COMMON_NS
{
	
template <typename T>
class LockFreeQueue {
public:

	LockFreeQueue() : m_Size(0), m_Head(new Node()), m_Tail(m_Head) { }

	~LockFreeQueue() {
		while (DequeueAndDelete());
		delete m_Head;
	}

	bool IsEmpty() const {
		return m_Head->m_Next == nullptr;
	}

	void Enqueue(const T& v) {
		const Node* const node = new Node(v);
		while (1) {
			const Node* const last = m_Tail;
			const Node* const next = last->m_Next;
			if (last != m_Tail) { continue; }
			if (next == nullptr) {
				if (compare_and_set(&last->m_Next, next, node)) {
					compare_and_set(&m_Tail, last, node);
					++m_Size;
					return;
				}
			}
			else {
				compare_and_set(&m_Tail, last, next);
			}
		}
	}

	T Dequeue() { // it returns false if there is nothing to deque.
		while (1) {
			const Node* const first = m_Head;
			const Node* const last = m_Tail;
			const Node* const next = first->m_Next;

			if (first != m_Head) { continue; }

			if (first == last) {
				if (next == nullptr) {
					while (m_Head->m_Next == nullptr) {
						usleep(1);
					}
					continue;
					//return false;
				}
				compare_and_set(&m_Tail, last, next);
			}
			else {
				T result = next->mValue;
				if (compare_and_set(&m_Head, first, next)) {
					delete first;
					return result;
				}
			}
		}
	}

	bool DequeueAndDelete() { // it destroys dequed item. but fast.
		while (1) {
			const Node* const first = m_Head;
			const Node* const last = m_Tail;
			const Node* const next = first->m_Next;

			if (first != m_Head) { continue; }

			if (first == last) {
				if (next == nullptr) {
					return false;
				}
				compare_and_set(&m_Tail, last, next);
			}
			else {
				if (compare_and_set(&m_Head, first, next)) {
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