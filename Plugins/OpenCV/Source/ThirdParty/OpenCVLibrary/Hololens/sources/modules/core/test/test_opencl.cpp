// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "test_precomp.hpp"
#include "opencv2/ts/ocl_test.hpp"

namespace opencv_test {
namespace ocl {

static void executeUMatCall(bool requireOpenCL = true)
{
    UMat a(100, 100, CV_8UC1, Scalar::all(0));
    UMat b;
    cv::add(a, Scalar::all(1), b);
    Mat b_cpu = b.getMat(ACCESS_READ);
    EXPECT_EQ(0, cv::norm(b_cpu - 1, NORM_INF));

    if (requireOpenCL)
    {
        EXPECT_TRUE(cv::ocl::useOpenCL());
    }
}

TEST(OCL_Context, createFromDevice)
{
    bool useOCL = cv::ocl::useOpenCL();

    OpenCLExecutionContext ctx = OpenCLExecutionContext::getCurrent();

    if (!useOCL)
    {
        ASSERT_TRUE(ctx.empty());  // Other tests should not broke global state
        throw SkipTestException("OpenCL is not available / disabled");
    }

    ASSERT_FALSE(ctx.empty());

    ocl::Device device = ctx.getDevice();
    ASSERT_FALSE(device.empty());

    ocl::Context context = ocl::Context::fromDevice(device);
    ocl::Context context2 = ocl::Context::fromDevice(device);

    EXPECT_TRUE(context.getImpl() == context2.getImpl()) << "Broken cache for OpenCL context (device)";
}

TEST(OCL_OpenCLExecutionContext, basic)
{
    bool useOCL = cv::ocl::useOpenCL();

    OpenCLExecutionContext ctx = OpenCLExecutionContext::getCurrent();

    if (!useOCL)
    {
        ASSERT_TRUE(ctx.empty());  // Other tests should not broke global state
        throw SkipTestException("OpenCL is not available / disabled");
    }

    ASSERT_FALSE(ctx.empty());

    ocl::Context context = ctx.getContext();
    ocl::Context context2 = ocl::Context::getDefault();
    EXPECT_TRUE(context.getImpl() == context2.getImpl());

    ocl::Device device = ctx.getDevice();
    ocl::Device device2 = ocl::Device::getDefault();
    EXPECT_TRUE(device.getImpl() == device2.getImpl());

    ocl::Queue queue = ctx.getQueue();
    ocl::Queue queue2 = ocl::Queue::getDefault();
    EXPECT_TRUE(queue.getImpl() == queue2.getImpl());
}

TEST(OCL_OpenCLExecutionContext, createAndBind)
{
    bool useOCL = cv::ocl::useOpenCL();

    OpenCLExecutionContext ctx = OpenCLExecutionContext::getCurrent();

    if (!useOCL)
    {
        ASSERT_TRUE(ctx.empty());  // Other tests should not broke global state
        throw SkipTestException("OpenCL is not available / disabled");
    }

    ASSERT_FALSE(ctx.empty());

    ocl::Context context = ctx.getContext();
    ocl::Device device = ctx.getDevice();

    OpenCLExecutionContext ctx2 = OpenCLExecutionContext::create(context, device);
    ASSERT_FALSE(ctx2.empty());

    try
    {
        ctx2.bind();
        executeUMatCall();
        ctx.bind();
        executeUMatCall();
    }
    catch (...)
    {
        ctx.bind();  // restore
        throw;
    }
}

TEST(OCL_OpenCLExecutionContext, createGPU)
{
    bool useOCL = cv::ocl::useOpenCL();

    OpenCLExecutionContext ctx = OpenCLExecutionContext::getCurrent();

    if (!useOCL)
    {
        ASSERT_TRUE(ctx.empty());  // Other tests should not broke global state
        throw SkipTestException("OpenCL is not available / disabled");
    }

    ASSERT_FALSE(ctx.empty());

    ocl::Context context = ocl::Context::create(":GPU:1");
    if (context.empty())
    {
        context = ocl::Context::create(":CPU:");
        if (context.empty())
            throw SkipTestException("OpenCL GPU1/CPU devices are not available");
    }

    ocl::Device device = context.device(0);

    OpenCLExecutionContext ctx2 = OpenCLExecutionContext::create(context, device);
    ASSERT_FALSE(ctx2.empty());

    try
    {
        ctx2.bind();
        executeUMatCall();
        ctx.bind();
        executeUMatCall();
    }
    catch (...)
    {
        ctx.bind();  // restore
        throw;
    }
}

TEST(OCL_OpenCLExecutionContext, ScopeTest)
{
    bool useOCL = cv::ocl::useOpenCL();

    OpenCLExecutionContext ctx = OpenCLExecutionContext::getCurrent();

    if (!useOCL)
    {
        ASSERT_TRUE(ctx.empty());  // Other tests should not broke global state
        throw SkipTestException("OpenCL is not available / disabled");
    }

    ASSERT_FALSE(ctx.empty());

    ocl::Context context = ocl::Context::create(":GPU:1");
    if (context.empty())
    {
        context = ocl::Context::create(":CPU:");
        if (context.empty())
            context = ctx.getContext();
    }

    ocl::Device device = context.device(0);

    OpenCLExecutionContext ctx2 = OpenCLExecutionContext::create(context, device);
    ASSERT_FALSE(ctx2.empty());

    try
    {
        OpenCLExecutionContextScope ctx_scope(ctx2);
        executeUMatCall();
    }
    catch (...)
    {
        ctx.bind();  // restore
        throw;
    }

    executeUMatCall();
}

} } // namespace opencv_test::ocl
