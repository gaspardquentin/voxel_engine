#include <gtest/gtest.h>
#include "voxel_engine/server/command_registry.h"
#include "voxel_engine/server/world.h"
#include "voxel_engine/server/chunk_generators.h"
#include "voxel_engine/network/i_server_connection.h"
#include "voxel_engine/network/client_event.h"
#include "voxel_engine/network/server_request.h"

using namespace voxeng::server;
using namespace voxeng::network;
using namespace voxeng;

// Minimal mock connection — just captures pushed events
class MockServerConnection : public IServerConnection {
public:
    std::vector<ClientEvent> pushed_events;
    void pushEvent(ClientEvent event) override { pushed_events.push_back(std::move(event)); }
    std::optional<ServerRequest> pollRequest() override { return std::nullopt; }
};

struct CommandCapture {
    bool called = false;
    std::vector<std::string> captured_args;
};

static CommandHandler makeCapture(CommandCapture& capture) {
    return [&capture](const std::vector<std::string>& args, CommandContext&) {
        capture.called = true;
        capture.captured_args = args;
    };
}

class CommandRegistryTest : public ::testing::Test {
protected:
    MockServerConnection mock_conn;
    // World with FlatGenerator, generate_chunks=false for speed
    World world{mock_conn, DEFAULT_VOXEL_TYPES, 42, std::make_unique<FlatGenerator>(), false};
    CommandRegistry registry;

    CommandContext makeContext() {
        return CommandContext{world, 1, [](const std::string&){}, [](const std::string&){}};
    }
};

TEST_F(CommandRegistryTest, RegisteredCommandFound) {
    CommandCapture capture;
    registry.registerCommand("/test", makeCapture(capture));

    auto ctx = makeContext();
    EXPECT_TRUE(registry.tryExecute("/test", ctx));
    EXPECT_TRUE(capture.called);
}

TEST_F(CommandRegistryTest, UnregisteredCommandNotFound) {
    auto ctx = makeContext();
    EXPECT_FALSE(registry.tryExecute("/unknown", ctx));
}

TEST_F(CommandRegistryTest, ArgumentParsing) {
    CommandCapture capture;
    registry.registerCommand("/cmd", makeCapture(capture));

    auto ctx = makeContext();
    registry.tryExecute("/cmd arg1 arg2 arg3", ctx);

    ASSERT_EQ(capture.captured_args.size(), 3u);
    EXPECT_EQ(capture.captured_args[0], "arg1");
    EXPECT_EQ(capture.captured_args[1], "arg2");
    EXPECT_EQ(capture.captured_args[2], "arg3");
}

TEST_F(CommandRegistryTest, EmptyInput) {
    auto ctx = makeContext();
    EXPECT_FALSE(registry.tryExecute("", ctx));
}

TEST_F(CommandRegistryTest, InputWithoutPrefix) {
    auto ctx = makeContext();
    EXPECT_FALSE(registry.tryExecute("hello", ctx));
}

TEST_F(CommandRegistryTest, CommandWithNoArgs) {
    CommandCapture capture;
    registry.registerCommand("/solo", makeCapture(capture));

    auto ctx = makeContext();
    registry.tryExecute("/solo", ctx);

    EXPECT_TRUE(capture.called);
    EXPECT_TRUE(capture.captured_args.empty());
}

TEST_F(CommandRegistryTest, MultipleCommandsRegistered) {
    CommandCapture capture_a, capture_b;
    registry.registerCommand("/a", makeCapture(capture_a));
    registry.registerCommand("/b", makeCapture(capture_b));

    auto ctx = makeContext();

    registry.tryExecute("/a", ctx);
    EXPECT_TRUE(capture_a.called);
    EXPECT_FALSE(capture_b.called);

    capture_a.called = false;
    registry.tryExecute("/b", ctx);
    EXPECT_TRUE(capture_b.called);
    EXPECT_FALSE(capture_a.called);
}

TEST_F(CommandRegistryTest, ErrorCallbackInvoked) {
    std::string error_msg;
    auto ctx = CommandContext{
        world, 1,
        [&error_msg](const std::string& msg) { error_msg = msg; },
        [](const std::string&) {}
    };

    registry.registerCommand("/err", [](const std::vector<std::string>&, CommandContext& c) {
        c.error("something went wrong");
    });

    registry.tryExecute("/err", ctx);
    EXPECT_EQ(error_msg, "something went wrong");
}
