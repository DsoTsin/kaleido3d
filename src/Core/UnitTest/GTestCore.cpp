#include "CoreMinimal.h"
#include <gtest/gtest.h>
//#include <KTL/HashMap.h>

#if K3DPLATFORM_OS_WINDOWS
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;

TEST(os, path)
{
    ASSERT_TRUE( os::MakeDir("GoogleTest") );
    ASSERT_TRUE( os::Remove("GoogleTest") );
}

TEST(os, sys)
{
    EXPECT_GE(os::GetCpuCoreNum(), 1U);
    EXPECT_GE(os::GetGpuCount(), 1U);
}

TEST(os, thread)
{
    auto file = MakeShared<os::File>();
    auto thread = MakeSharedMacro(os::Thread, [file]()
    {
        os::Sleep(1000);
    }, "Test Threading");
    thread->Join();
    EXPECT_EQ(file.UseCount(), 1);
}

TEST(core, string)
{
    String testString("HeyYou!");
    EXPECT_EQ(testString.Length(), 7);
    EXPECT_EQ(testString.FindFirstOf("Y"), 3);
    EXPECT_EQ(testString.Find("Ys"), String::npos);
    EXPECT_EQ(testString.Find("Yo"), 3);
    EXPECT_EQ(testString.FindLastOf("y"), 2);
    EXPECT_EQ(testString.FindLastNotOf("u"), 6);

    testString.AppendSprintf("%.2f %s", 1.5f, "Soo");
    EXPECT_EQ(testString, String("HeyYou!1.50 Soo"));

    auto testMd5 = MD5Encode(testString);
    EXPECT_EQ(String("a27be65dfc722bad5cb07751ac19ed07"), testMd5);

    auto base64 = Base64Encode(testString);
    auto decode64 = Base64Decode(base64);
    EXPECT_EQ(decode64, testString);

    EXPECT_EQ(testString.SubStr(3, 3), String("You"));

    String testStringBaby("BabyGirl!!");
    testStringBaby.Swap(testString);
    testString.AppendSprintf("%d %s", 5, "fhdsfjdhjkfdhksfhdkjshfjkdshfk");
    testString += 'B';

    String testMoveString(Move(testMd5));
    EXPECT_TRUE(testMd5.CStr() == nullptr);
}

TEST(core, array)
{
    DynArray<int> ints;
    ints.Append(5).Append(6).Append(7).Append(0).Append(8);
    EXPECT_EQ(ints[0], 5);
    EXPECT_EQ(ints[4], 8);
    EXPECT_EQ(ints.Count(), 5);
    DynArray<String> strings;
    strings.Resize(7);
    strings[6] = "Right____";
    strings[3] = "Left...";
    strings.Append("kkkkk");
    EXPECT_EQ(strings[7], String("kkkkk"));
    EXPECT_TRUE(strings.Contains("Right____"));

    CircularBuffer<int> CBuffer(20);
    CircularQueue<int>  CQueue(20);
    EXPECT_EQ(CBuffer.Capacity(), 20);
    EXPECT_EQ(CQueue.Capacity(), 20);
}

TEST(core, sharedptr)
{
    class spTest : public EnableSharedFromThis<spTest>
    {
    public:
        spTest(int& _counter) : counter(_counter) {}
        ~spTest() { counter--; }

    private:
        int& counter;
    };
    int counter = 0;
    {
        auto pTest = MakeSharedMacro(spTest, counter);
    }
    EXPECT_EQ(-1, counter);
}

//TEST(core, regex)
//{
//    // Simple
//    RegEx reg("\"(.*)\"", RegEx::Default);
//    EXPECT_TRUE(reg.Match("ddff d=\"fhbdhfdhfh\""));
//    EXPECT_TRUE(!reg.Match("gfdgfgfg"));
//
//    // Group with all names
//    RegEx reg_group("\\[(?<key>(.*))\\]:\\s\\[(?<value>(.*))\\]\\n", RegEx::IgnoreCase);
//    RegEx::Groups matched_group;
//    EXPECT_TRUE(reg_group.Match("[ro.build.version.release]: [8.0.0]\n"
//        "[ro.build.version.sdk] : [26]\n"
//        "[ro.product.manufacturer] : [Xiaomi]\n"
//        "[ro.product.model] : [MI 6]\n", matched_group));
//
//    matched_group[0].SubGroup(0);
//    auto val = matched_group[0].SubGroup("key");
//
//    // Group 2 without names
//    RegEx reg_group2("\\[(.*)\\]:\\s\\[(.*)\\]\\n", RegEx::IgnoreCase);
//    RegEx::Groups matched_group2;
//    EXPECT_TRUE(reg_group2.Match("[ro.build.version.release]: [8.0.0]\n"
//        "[ro.build.version.sdk] : [26]\n"
//        "[ro.product.manufacturer] : [Xiaomi]\n"
//        "[ro.product.model] : [MI 6]\n", matched_group2));
//
//
//    // Group 3 without names
//    RegEx reg_group3("\\[(.*)\\]:\\s\\[(?<value>(.*))\\]\\n", RegEx::IgnoreCase);
//    RegEx::Groups matched_group3;
//    EXPECT_TRUE(reg_group3.Match("[ro.build.version.release]: [8.0.0]\n"
//        "[ro.build.version.sdk] : [26]\n"
//        "[ro.product.manufacturer] : [Xiaomi]\n"
//        "[ro.product.model] : [MI 6]\n", matched_group3, RegEx::MultiLine));
//    auto newVal = matched_group3[0].SubGroup("value");
//    EXPECT_EQ(String("8.0.0"), newVal);
//}

// ADB
class NetClient : public os::Socket
{
public:
    NetClient() : Socket(os::SockType::TCP)
    {
        Create();
        SetBlocking(true);
        os::IpAddress Lip("127.0.0.1");
        Lip.SetAddrPort(5037);
        Connect(Lip);
    }

    void OnHandleError(int Code) override
    {

    }

    bool Test()
    {
        char buffer[256] = {};
        const char* msg = "host:devices";
        snprintf(buffer, 256, "%04x%s", strlen(msg), msg);
        auto sent = Send(buffer, 16);
        EXPECT_EQ(sent, 16);
        auto Rcv = Receive(buffer, 256);
        if (Rcv == 4)
        {
            Rcv = Receive(buffer, 256);
        }
        return Rcv > 0;
    }
};

TEST(core, net)
{
    NetClient sock;
    sock.Test();
    EXPECT_EQ(1, 1);
}

TEST(core, simd)
{
    V4F data = simd::MakeFloat4(1.0f, 0.5f, 1.0f, 0.1f);
    data = simd::Add(data, data);
    data = simd::Max(data, simd::MakeFloat4(1.0f));
    EXPECT_EQ(simd::GetFloat(data, 0), 2.0f);
}

TEST(core, math)
{
    math::TVector<float, 4> Vector4 = {1.0f, 0.4f, 0.2f, 1.0f};
    math::TMatrixN<float, 4> Mat4;
    Vector4 = Mat4 * Vector4;
}

TEST(core, lockfree_queue)
{
    k3d::LockFreeQueue<String> queue;
    queue.Enqueue("dsdsdgv");
    String Info;
    queue.Dequeue(Info);
    EXPECT_EQ(7, Info.Length());
}

struct ZTile
{
    V4F ZMin[2];
    V4I Mask;
};

struct MaskedOcclusionCulling
{
    ZTile* HiZBuffer;
    
    int Width;
    int Height;

    int TileWidth;
    int TileHeight;

    float Near;

private:

    void Init()
    {
        HiZBuffer = (ZTile*)GetDefaultAllocator().Alloc(sizeof(ZTile) * TileWidth * TileHeight, 64);
    }

    /*
     * V4F (x,y,z, w (reserved for depth))
     */
    void RenderTriangles(const V4F* Vertices, int NumTris, math::TMatrixN<float, 4> const& ClipMatrix)
    {
        // Initialize Layers

        // Project Triangles

        // Cull Back Faces

        // Rasterize Triangles

        // Fetch Results
    }

    void ProjectVertex(const V4F* InVert, math::TMatrixN<float, 4> const& ClipMatrix, V4F* Projected)
    {

    }

    void RasterizeTriangle()
    {
        // Compute BoundingBox (Rect)

        // Sort Vertices

        // Compute Edges

        // Perform Scanline 
    }

    void Destroy()
    {
        if (HiZBuffer)
        {
            GetDefaultAllocator().DeAlloc(HiZBuffer);
            HiZBuffer = nullptr;
        }
    }
};

TEST(core, geomath)
{
    math::AABB aabb;
    math::AABB xxyy;
    math::Sphere sphere;
    math::Plane plane;
    aabb |= xxyy;
    EXPECT_EQ(sizeof(aabb), 32);
    EXPECT_EQ(sizeof(sphere), 16);
    EXPECT_EQ(sizeof(plane), 16);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
