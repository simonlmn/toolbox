#ifndef TOOLBOX_REPOSITORY_H_
#define TOOLBOX_REPOSITORY_H_

namespace toolbox
{

class IRepository {
public:
  virtual void commit() = 0;
  virtual void rollback() = 0;
};

template<typename R>
class Transaction final {
  R* _repository;

public:
  Transaction() {}
  Transaction(R* repository) : _repository(repository) {}
  Transaction(const Transaction& other) = delete;
  Transaction& operator=(const Transaction& other) = delete;
  ~Transaction() {
    rollback();
  }

  void commit() {
    if (_repository) {
      _repository->commit();
      _repository = nullptr;
    }
  }

  void rollback() {
    if (_repository) {
      _repository->rollback();
      _repository = nullptr;
    }
  }
};

template<typename R>
Transaction<R> beginTransaction(R& repository) {
  return {&repository};
}

template<typename R>
Transaction<R> beginTransaction(R* repository) {
  return {repository};
}

}

#endif