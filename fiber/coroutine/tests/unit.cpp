#include "../source/coroutine.hpp"

#include <wheels/test/test_framework.hpp>

#include <memory>
#include <string>
#include <thread>
#include <sstream>

//////////////////////////////////////////////////////////////////////

struct TreeNode;

using TreeNodePtr = std::shared_ptr<TreeNode>;

struct TreeNode {
  TreeNodePtr left;
  TreeNodePtr right;
  std::string data;

  TreeNode(std::string d, TreeNodePtr l, TreeNodePtr r)
      : left(std::move(l)),
        right(std::move(r)),
        data(std::move(d)) {
  }

  static TreeNodePtr Branch(std::string data, TreeNodePtr left, TreeNodePtr right) {
    return std::make_shared<TreeNode>(
        std::move(data),
        std::move(left),
        std::move(right));
  }

  static TreeNodePtr Leaf(std::string data) {
    return std::make_shared<TreeNode>(
        std::move(data), nullptr, nullptr);
  }
};

//////////////////////////////////////////////////////////////////////

class TreeIterator {
 public:
  explicit TreeIterator(TreeNodePtr root)
      : walker_([this, root](Coroutine::SuspendContext self) {
          TreeWalk(root, self);
        }) {
  }

  bool TryNext() {
    walker_.Resume();
    return !walker_.IsDone();
  }

  std::string_view Data() const {
    return data_;
  }

 private:
  void TreeWalk(TreeNodePtr node, auto ctx) {
    if (node->left) {
      TreeWalk(node->left, ctx);
    }

    data_ = node->data;
    ctx.Suspend();

    if (node->right) {
      TreeWalk(node->right, ctx);
    }
  }

 private:
  Coroutine walker_;
  std::string_view data_;
};

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Coroutine) {
  SIMPLE_TEST(Suspend) {
    int step = 0;

    Coroutine coro([&](auto self) {
      ++step;
      self.Suspend();
      ++step;
    });

    ASSERT_EQ(step, 0);
    ASSERT_FALSE(coro.IsDone());

    coro.Resume();

    ASSERT_EQ(step, 1);
    ASSERT_FALSE(coro.IsDone());

    coro.Resume();

    ASSERT_EQ(step, 2);
    ASSERT_TRUE(coro.IsDone());
  }

  SIMPLE_TEST(SuspendForLoop) {
    const size_t kIters = 128;

    Coroutine coro([](auto self) {
      for (size_t i = 0; i < kIters; ++i) {
        self.Suspend();
      }
    });

    for (size_t i = 0; i < kIters; ++i) {
      coro.Resume();
    }

    ASSERT_FALSE(coro.IsDone());

    coro.Resume();  // Last step

    ASSERT_TRUE(coro.IsDone());
  }

  SIMPLE_TEST(Interleaving) {
    int step = 0;

    Coroutine a([&](auto self) {
      ASSERT_EQ(step, 0);
      step = 1;
      self.Suspend();
      ASSERT_EQ(step, 2);
      step = 3;
    });

    Coroutine b([&](auto self) {
      ASSERT_EQ(step, 1);
      step = 2;
      self.Suspend();
      ASSERT_EQ(step, 3);
      step = 4;
    });

    a.Resume();
    b.Resume();

    ASSERT_EQ(step, 2);

    a.Resume();
    b.Resume();

    ASSERT_TRUE(a.IsDone());
    ASSERT_TRUE(b.IsDone());

    ASSERT_EQ(step, 4);
  }

  struct Threads {
    template <typename F>
    void Run(F task) {
      std::thread t([task = std::move(task)]() mutable {
        task();
      });
      t.join();
    }
  };

  SIMPLE_TEST(Threads) {
    size_t steps = 0;

    Coroutine coro([&steps](auto self) {
      ++steps;
      self.Suspend();
      ++steps;
      self.Suspend();
      ++steps;
    });

    auto step = [&coro]() {
      coro.Resume();
    };

    // Simulate fiber running on thread pool
    Threads threads;

    threads.Run(step);
    ASSERT_EQ(steps, 1);

    threads.Run(step);
    ASSERT_EQ(steps, 2);

    threads.Run(step);
    ASSERT_EQ(steps, 3);
  }

  void TreeWalk(TreeNodePtr node, auto ctx) {
    if (node->left) {
      TreeWalk(node->left, ctx);
    }

    ctx.Suspend();

    if (node->right) {
      TreeWalk(node->right, ctx);
    }
  }

  SIMPLE_TEST(TreeWalk) {
    auto tree = TreeNode::Branch(
        "B",
        TreeNode::Leaf("A"),
        TreeNode::Branch(
            "F",
            TreeNode::Branch(
                "D",
                TreeNode::Leaf("C"),
                TreeNode::Leaf("E")),
            TreeNode::Leaf("G")));

    std::stringstream traversal;

    TreeIterator iter(tree);
    while (iter.TryNext()) {
      traversal << iter.Data();
    }

    ASSERT_EQ(traversal.str(), "ABCDEFG");
  }

  SIMPLE_TEST(Pipeline) {
    const size_t kSteps = 123;

    size_t steps = 0;

    Coroutine outer([&](auto self) {
      Coroutine inner([&steps](auto self) {
        for (size_t i = 0; i < kSteps; ++i) {
          ++steps;
          self.Suspend();
        }
      });

      while (!inner.IsDone()) {
        inner.Resume();
        self.Suspend();
      }
    });

    while (!outer.IsDone()) {
      outer.Resume();
    }

    ASSERT_EQ(steps, kSteps);
  }

  SIMPLE_TEST(Leak) {
    struct Widget {};

    auto strong_ref = std::make_shared<Widget>();
    std::weak_ptr<Widget> weak_ref = strong_ref;

    {
      Coroutine coro([ref = std::move(strong_ref)](auto /*self*/) {
        //
      });

      coro.Resume();
    }

    ASSERT_FALSE(weak_ref.lock());
  }
}

RUN_ALL_TESTS()
