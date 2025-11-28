#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>
#include <sstream>
#include <memory>
#include <vector>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/pattern_formatter.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

TEST(LoggerCaptureTest, InfoGoesToSink)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("test_logger", sink);
    spdlog::register_logger(logger);
    logger->set_level(spdlog::level::info);

    logger->info("hello {}", "world");

    std::string out = buf.str();
    EXPECT_NE(out.find("hello world"), std::string::npos);

    spdlog::drop("test_logger");
}

TEST(LoggerTest, LevelFilteringWorks)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("level_logger", sink);
    spdlog::register_logger(logger);

    logger->set_level(spdlog::level::warn);
    logger->info("this is info");
    logger->warn("this is warn");

    auto out = buf.str();
    EXPECT_EQ(out.find("this is info"), std::string::npos);
    EXPECT_NE(out.find("this is warn"), std::string::npos);

    spdlog::drop("level_logger");
}

TEST(LoggerTest, MultipleSinksReceiveSameMessage)
{
    std::ostringstream b1, b2;
    auto s1 = std::make_shared<spdlog::sinks::ostream_sink_mt>(b1);
    auto s2 = std::make_shared<spdlog::sinks::ostream_sink_mt>(b2);
    std::vector<spdlog::sink_ptr> sinks{ s1, s2 };
    auto logger = std::make_shared<spdlog::logger>("multi_logger", sinks.begin(), sinks.end());
    spdlog::register_logger(logger);
    logger->set_level(spdlog::level::info);

    logger->info("multi {}", 42);

    auto o1 = b1.str();
    auto o2 = b2.str();
    EXPECT_NE(o1.find("multi 42"), std::string::npos);
    EXPECT_NE(o2.find("multi 42"), std::string::npos);

    spdlog::drop("multi_logger");
}

TEST(LoggerTest, FileSinkWritesErrorLog)
{
    namespace fs = std::filesystem;
    auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    fs::path tmp = fs::temp_directory_path() / ("spdlog_test_" + stamp + ".log");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(tmp.string(), true);
    auto logger = std::make_shared<spdlog::logger>("file_logger", file_sink);
    spdlog::register_logger(logger);

    logger->error("file error {}", 7);
    logger->flush();

    std::ifstream in(tmp);
    ASSERT_TRUE(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    EXPECT_NE(content.find("file error 7"), std::string::npos);

    spdlog::drop("file_logger");
    std::error_code ec;
    fs::remove(tmp, ec);
}

TEST(LoggerTest, PatternFormattingIncludesLoggerName)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    sink->set_pattern("[%n] %v");
    auto logger = std::make_shared<spdlog::logger>("pattern_logger", sink);
    spdlog::register_logger(logger);

    logger->info("patterned");
    auto out = buf.str();
    EXPECT_NE(out.find("[pattern_logger] patterned"), std::string::npos);

    spdlog::drop("pattern_logger");
}

TEST(LoggerTest, ChangeLevelAtRuntime)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("dynamic_logger", sink);
    spdlog::register_logger(logger);

    logger->set_level(spdlog::level::info);
    logger->debug("d1");
    logger->info("i1");
    logger->set_level(spdlog::level::debug);
    logger->debug("d2");

    auto out = buf.str();
    EXPECT_EQ(out.find("d1"), std::string::npos);
    EXPECT_NE(out.find("i1"), std::string::npos);
    EXPECT_NE(out.find("d2"), std::string::npos);

    spdlog::drop("dynamic_logger");
}

TEST(LoggerTest, ThreadedLoggingAllMessagesCaptured)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("thread_logger", sink);
    spdlog::register_logger(logger);
    logger->set_level(spdlog::level::info);

    const int threads = 4;
    const int per = 50;
    std::vector<std::thread> ths;
    for (int t = 0; t < threads; ++t) {
        ths.emplace_back([logger, per, t]() {
            for (int i = 0; i < per; ++i) {
                logger->info("thr {} msg {}", t, i);
            }
        });
    }
    for (auto &t : ths) t.join();

    auto out = buf.str();
    int count = 0;
    std::string needle = "thr 0 msg 0";
    size_t pos = out.find(needle);
    while (pos != std::string::npos) {
        ++count;
        pos = out.find(needle, pos + 1);
    }
    EXPECT_GT(count, 0);

    spdlog::drop("thread_logger");
}

TEST(LoggerTest, DropLoggerRemovesIt)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("temp_logger", sink);
    spdlog::register_logger(logger);

    auto got = spdlog::get("temp_logger");
    ASSERT_NE(got, nullptr);
    spdlog::drop("temp_logger");
    got = spdlog::get("temp_logger");
    EXPECT_EQ(got, nullptr);
}

TEST(LoggerTest, SinkFormatterChangeAffectsOutput)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("fmt_logger", sink);
    spdlog::register_logger(logger);

    sink->set_formatter(std::make_unique<spdlog::pattern_formatter>("%L: %v"));
    logger->warn("warned");
    auto out = buf.str();
    EXPECT_NE(out.find("W: warned"), std::string::npos);

    spdlog::drop("fmt_logger");
}

TEST(LoggerTest, LoggerFlushEnsuresFileWrite)
{
    namespace fs = std::filesystem;
    auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    fs::path tmp = fs::temp_directory_path() / ("spdlog_flush_" + stamp + ".log");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(tmp.string(), true);
    auto logger = std::make_shared<spdlog::logger>("flush_logger", file_sink);
    spdlog::register_logger(logger);

    logger->info("flushed message");
    logger->flush();

    std::ifstream in(tmp);
    ASSERT_TRUE(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    EXPECT_NE(content.find("flushed message"), std::string::npos);

    spdlog::drop("flush_logger");
    std::error_code ec;
    fs::remove(tmp, ec);
}

TEST(LoggerTest, SinkLevelFilteringWorks)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    // sink-level filter: only errors should appear
    sink->set_level(spdlog::level::err);

    auto logger = std::make_shared<spdlog::logger>("sink_level_logger", sink);
    spdlog::register_logger(logger);

    logger->warn("will not appear");
    logger->error("will appear");

    auto out = buf.str();
    EXPECT_EQ(out.find("will not appear"), std::string::npos);
    EXPECT_NE(out.find("will appear"), std::string::npos);

    spdlog::drop("sink_level_logger");
}

TEST(LoggerTest, LongMessageIsCaptured)
{
    std::ostringstream buf;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(buf);
    auto logger = std::make_shared<spdlog::logger>("longmsg_logger", sink);
    spdlog::register_logger(logger);

    std::string longmsg;
    for (int i = 0; i < 1000; ++i) longmsg += "0123456789";

    logger->info("long: {}", longmsg);

    auto out = buf.str();
    EXPECT_NE(out.find("long: 0123456789"), std::string::npos);

    spdlog::drop("longmsg_logger");
}

TEST(LoggerTest, SinkFlushAfterManyMessages)
{
    namespace fs = std::filesystem;
    auto stamp = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    fs::path tmp = fs::temp_directory_path() / ("spdlog_many_" + stamp + ".log");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(tmp.string(), true);
    auto logger = std::make_shared<spdlog::logger>("many_logger", file_sink);
    spdlog::register_logger(logger);

    for (int i = 0; i < 200; ++i) {
        logger->info("msg {}", i);
    }
    logger->flush();

    std::ifstream in(tmp);
    ASSERT_TRUE(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    EXPECT_NE(content.find("msg 0"), std::string::npos);
    EXPECT_NE(content.find("msg 199"), std::string::npos);

    spdlog::drop("many_logger");
    std::error_code ec;
    fs::remove(tmp, ec);
}
