template<typename T>
struct fifo_node
{
  struct fifo_node *next;
  T value;
};

template<typename T>
class fifo
{
  fifo_node<T> *front;
  fifo_node<T> *back;

  fifo_node<T> *dequeue(void)
  {
    fifo_node<T> *tmp = front;
    front = front->next;
    return tmp;
  }

  void enqueue(fifo_node<T> value)
  {
    fifo_node<T> *tempNode = new fifo_node<T>;
    tempNode->value = value;
    back->next = tempNode;
    back = tempNode;
  }
};

int main()
{
    fifo<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);

    cout << queue.dequeue() << ", \n";
    cout << queue.dequeue() << ", \n";
    cout << queue.dequeue() << ", \n";
    return 0;
}