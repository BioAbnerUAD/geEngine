#include <vld.h>
#include <gtest/gtest.h>
#include <gePrerequisitesUtil.h>
#include <geMath.h>
#include <geFileSystem.h>
#include <geDataStream.h>

using namespace geEngineSDK;

TEST(geUtility, DEFINED_TYPES_SIZES) {
  ASSERT_TRUE(sizeof(uint8  ) == 1);
  ASSERT_TRUE(sizeof(uint16 ) == 2);
  ASSERT_TRUE(sizeof(uint32 ) == 4);
  ASSERT_TRUE(sizeof(uint64 ) == 8);
  ASSERT_TRUE(sizeof(uint128) == 16);
  
  ASSERT_TRUE(sizeof(int8  ) == 1);
  ASSERT_TRUE(sizeof(int16 ) == 2);
  ASSERT_TRUE(sizeof(int32 ) == 4);
  ASSERT_TRUE(sizeof(int64 ) == 8);
  ASSERT_TRUE(sizeof(int128) == 16);

  ASSERT_TRUE(sizeof(float ) == 4);
  ASSERT_TRUE(sizeof(double) == 8);

  ASSERT_TRUE(static_cast<uint32>(-1) == Math::MAX_UINT32);
}

TEST(geUtility, String) {
  String testString = "This is a test";
  ASSERT_TRUE(testString.size() == 14);

  WString testWString = L"This is a test";
  ASSERT_TRUE(testWString.size() == 14);

  ASSERT_TRUE(toString(testWString).size() == 14);
  ASSERT_TRUE(toWString(testString).size() == 14);
}

TEST(geUtility, Path) {
  Path testPath;
  WString lastDirectory;
  
  testPath = FileSystem::GetWorkingDirectoryPath();
  ASSERT_TRUE(testPath.isDirectory());
  ASSERT_TRUE(testPath.getNumDirectories());

  lastDirectory.append(testPath[testPath.getNumDirectories() - 1]);
  ASSERT_TRUE(lastDirectory.compare(L"BIN"));
}

TEST(geUtility, Parser) {
  DataStreamPtr fileData = FileSystem::OpenFile("Test/test.txt");
  ASSERT_TRUE(fileData);
  if (fileData) {
    String strParse = fileData->GetAsString();
    Vector<String> lineList = StringUtil::split(strParse, "\n");

    for (auto& line : lineList) {
      StringUtil::trim(line);
    }

    ASSERT_TRUE(lineList.size() == 9);
  }
}