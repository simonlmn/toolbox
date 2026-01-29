#ifndef TOOLBOX_REPOSITORY_H_
#define TOOLBOX_REPOSITORY_H_

namespace toolbox
{

/**
 * Minimal repository interface for transactional operations.
 */
class IRepository {
public:
  virtual void commit() = 0;
  virtual void rollback() = 0;
};

/**
 * Scoped transaction helper that auto-rolls back unless committed.
 */
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

/**
 * Begin a transaction for a repository reference.
 */
template<typename R>
Transaction<R> beginTransaction(R& repository) {
  return {&repository};
}

/**
 * Begin a transaction for a repository pointer (may be null).
 */
template<typename R>
Transaction<R> beginTransaction(R* repository) {
  return {repository};
}

}

#endif