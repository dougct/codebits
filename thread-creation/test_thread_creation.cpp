#include <gtest/gtest.h>
#include <thread>
#include <future>
#include <atomic>

class ThreadCreationTest : public ::testing::Test {
protected:
    std::atomic<size_t> counter{0};

    void SetUp() override {
        counter = 0;
    }
};

TEST_F(ThreadCreationTest, EmptyThreadCreation) {
    auto thread = std::thread([] { });
    EXPECT_TRUE(thread.joinable());
    thread.join();
}

TEST_F(ThreadCreationTest, ThreadWithCounter) {
    auto thread = std::thread([this] { counter++; });
    thread.join();
    EXPECT_EQ(counter, 1);
}

TEST_F(ThreadCreationTest, AsyncWithCounter) {
    auto future = std::async(std::launch::async, [this]{counter++;});
    future.get();
    EXPECT_EQ(counter, 1);
}

TEST_F(ThreadCreationTest, MultipleThreads) {
    const int num_threads = 10;
    std::vector<std::thread> threads;

    // Create multiple threads
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back([this] { counter++; });
    }

    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(counter, num_threads);
}

TEST_F(ThreadCreationTest, MultipleAsync) {
    const int num_tasks = 10;
    std::vector<std::future<void>> futures;

    // Create multiple async tasks
    for (int i = 0; i < num_tasks; i++) {
        futures.push_back(std::async(std::launch::async, [this]{counter++;}));
    }

    // Wait for all tasks
    for (auto& future : futures) {
        future.get();
    }

    EXPECT_EQ(counter, num_tasks);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
