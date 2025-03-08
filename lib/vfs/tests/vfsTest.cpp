#include <gtest/gtest.h>
#include "vfs.hpp"
#include <filesystem>
#include <fstream>
#include <string>

class VFSTest : public ::testing::Test {
protected:
    // Test directory for persistence
    std::string test_dir;
    
    void SetUp() override {
        // Create a unique test directory
        test_dir = "vfs_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        std::filesystem::create_directory(test_dir);
    }
    
    void TearDown() override {
        // Clean up the test directory after the test
        std::filesystem::remove_all(test_dir);
    }
    
    // Helper function to create a simple text file for testing
    std::vector<unsigned char> createTestData(const std::string& content) {
        std::vector<unsigned char> data(content.begin(), content.end());
        return data;
    }
};

TEST_F(VFSTest, AddAndRetrieveFile) {
    VirtualFileSystem vfs;
    
    // Create test data
    std::string content = "Hello, VFS!";
    std::vector<unsigned char> data = createTestData(content);
    
    // Add file to VFS
    vfs.add_file("test.txt", data.data(), data.size());
    
    // Check if file exists
    EXPECT_TRUE(vfs.exists("test.txt"));
    
    // Retrieve and check the file
    const VirtualFileSystem::FileEntry* entry = vfs.get_file("test.txt");
    ASSERT_NE(entry, nullptr);
    
    // Convert back to string for comparison
    std::string retrieved_content(entry->data.begin(), entry->data.end());
    EXPECT_EQ(retrieved_content, content);
}

TEST_F(VFSTest, FileDoesNotExist) {
    VirtualFileSystem vfs;
    
    // Check for a file that doesn't exist
    EXPECT_FALSE(vfs.exists("nonexistent.txt"));
    
    // Try to retrieve a non-existent file
    const VirtualFileSystem::FileEntry* entry = vfs.get_file("nonexistent.txt");
    EXPECT_EQ(entry, nullptr);
}

TEST_F(VFSTest, MimeTypeDetection) {
    VirtualFileSystem vfs;
    std::vector<unsigned char> data = createTestData("Test content");
    
    // Add files with different extensions
    vfs.add_file("test.html", data.data(), data.size());
    vfs.add_file("test.css", data.data(), data.size());
    vfs.add_file("test.js", data.data(), data.size());
    vfs.add_file("test.png", data.data(), data.size());
    vfs.add_file("test.jpg", data.data(), data.size());
    vfs.add_file("test.jpeg", data.data(), data.size());
    vfs.add_file("test.svg", data.data(), data.size());
    vfs.add_file("test.json", data.data(), data.size());
    vfs.add_file("test.wasm", data.data(), data.size());
    vfs.add_file("test.zip", data.data(), data.size());
    vfs.add_file("test.bin", data.data(), data.size());
    
    // Check mime types
    EXPECT_EQ(vfs.get_file("test.html")->mime_type, "text/html");
    EXPECT_EQ(vfs.get_file("test.css")->mime_type, "text/css");
    EXPECT_EQ(vfs.get_file("test.js")->mime_type, "application/javascript");
    EXPECT_EQ(vfs.get_file("test.png")->mime_type, "image/png");
    EXPECT_EQ(vfs.get_file("test.jpg")->mime_type, "image/jpeg");
    EXPECT_EQ(vfs.get_file("test.jpeg")->mime_type, "image/jpeg");
    EXPECT_EQ(vfs.get_file("test.svg")->mime_type, "image/svg+xml");
    EXPECT_EQ(vfs.get_file("test.json")->mime_type, "application/json");
    EXPECT_EQ(vfs.get_file("test.wasm")->mime_type, "application/wasm");
    EXPECT_EQ(vfs.get_file("test.zip")->mime_type, "application/zip");
    EXPECT_EQ(vfs.get_file("test.bin")->mime_type, "application/octet-stream");
}

TEST_F(VFSTest, BasicPersistence) {
    std::string content = "Persistence test data";
    std::vector<unsigned char> data = createTestData(content);
    
    // First VFS instance - create and save a file
    {
        VirtualFileSystem vfs(test_dir);
        EXPECT_TRUE(vfs.is_persistent());
        
        vfs.add_file("persistent.txt", data.data(), data.size());
        EXPECT_TRUE(vfs.save_to_disk());
        
        // Verify the file was created on disk
        std::filesystem::path disk_path = std::filesystem::path(test_dir) / "persistent.txt";
        EXPECT_TRUE(std::filesystem::exists(disk_path));
    }
    
    // Second VFS instance - should load the file from disk
    {
        VirtualFileSystem vfs2(test_dir);
        EXPECT_TRUE(vfs2.exists("persistent.txt"));
        
        const VirtualFileSystem::FileEntry* entry = vfs2.get_file("persistent.txt");
        ASSERT_NE(entry, nullptr);
        
        std::string retrieved_content(entry->data.begin(), entry->data.end());
        EXPECT_EQ(retrieved_content, content);
    }
}

TEST_F(VFSTest, PersistenceWithSubdirectories) {
    std::string content = "Nested file content";
    std::vector<unsigned char> data = createTestData(content);
    
    // First VFS instance - create and save files in subdirectories
    {
        VirtualFileSystem vfs(test_dir);
        
        vfs.add_file("subdir/nested.txt", data.data(), data.size());
        vfs.add_file("deeply/nested/path/file.txt", data.data(), data.size());
        EXPECT_TRUE(vfs.save_to_disk());
        
        // Verify the directories and files were created on disk
        std::filesystem::path nested_path = std::filesystem::path(test_dir) / "subdir" / "nested.txt";
        std::filesystem::path deep_path = std::filesystem::path(test_dir) / "deeply" / "nested" / "path" / "file.txt";
        
        EXPECT_TRUE(std::filesystem::exists(nested_path));
        EXPECT_TRUE(std::filesystem::exists(deep_path));
    }
    
    // Second VFS instance - should load all files from disk
    {
        VirtualFileSystem vfs2(test_dir);
        EXPECT_TRUE(vfs2.exists("subdir/nested.txt"));
        EXPECT_TRUE(vfs2.exists("deeply/nested/path/file.txt"));
        
        const VirtualFileSystem::FileEntry* entry1 = vfs2.get_file("subdir/nested.txt");
        ASSERT_NE(entry1, nullptr);
        
        const VirtualFileSystem::FileEntry* entry2 = vfs2.get_file("deeply/nested/path/file.txt");
        ASSERT_NE(entry2, nullptr);
        
        std::string retrieved_content1(entry1->data.begin(), entry1->data.end());
        std::string retrieved_content2(entry2->data.begin(), entry2->data.end());
        
        EXPECT_EQ(retrieved_content1, content);
        EXPECT_EQ(retrieved_content2, content);
    }
}

TEST_F(VFSTest, NonPersistentVFS) {
    VirtualFileSystem vfs; // Non-persistent by default
    
    EXPECT_FALSE(vfs.is_persistent());
    
    // Add a file
    std::string content = "Non-persistent data";
    std::vector<unsigned char> data = createTestData(content);
    vfs.add_file("nonpersistent.txt", data.data(), data.size());
    
    // Try to save to disk (should fail)
    EXPECT_FALSE(vfs.save_to_disk());
    
    // Set persistence directory and try again
    vfs.set_persistence_dir(test_dir);
    EXPECT_FALSE(vfs.save_to_disk()); // Should still fail because persistence was not enabled in constructor
}

TEST_F(VFSTest, BinaryFileHandling) {
    // Create binary data with zeros and non-printable characters
    std::vector<unsigned char> binary_data;
    for (int i = 0; i < 256; i++) {
        binary_data.push_back(static_cast<unsigned char>(i));
    }
    
    VirtualFileSystem vfs(test_dir);
    vfs.add_file("binary.dat", binary_data.data(), binary_data.size());
    
    // Check if file exists and retrieve it
    EXPECT_TRUE(vfs.exists("binary.dat"));
    const VirtualFileSystem::FileEntry* entry = vfs.get_file("binary.dat");
    ASSERT_NE(entry, nullptr);
    
    // Verify binary data is preserved
    EXPECT_EQ(entry->data.size(), binary_data.size());
    for (size_t i = 0; i < binary_data.size(); i++) {
        EXPECT_EQ(entry->data[i], binary_data[i]);
    }
    
    // Test persistence of binary data
    EXPECT_TRUE(vfs.save_to_disk());
    
    // Load in a new VFS instance
    VirtualFileSystem vfs2(test_dir);
    EXPECT_TRUE(vfs2.exists("binary.dat"));
    const VirtualFileSystem::FileEntry* entry2 = vfs2.get_file("binary.dat");
    ASSERT_NE(entry2, nullptr);
    
    // Verify binary data is preserved after persistence
    EXPECT_EQ(entry2->data.size(), binary_data.size());
    for (size_t i = 0; i < binary_data.size(); i++) {
        EXPECT_EQ(entry2->data[i], binary_data[i]);
    }
}

TEST_F(VFSTest, GetFilesList) {
    VirtualFileSystem vfs(test_dir);
    
    // Create test data
    std::string content = "Test content";
    std::vector<unsigned char> data = createTestData(content);
    
    // Add multiple files
    vfs.add_file("file1.txt", data.data(), data.size());
    vfs.add_file("file2.txt", data.data(), data.size());
    vfs.add_file("subdir/file3.txt", data.data(), data.size());
    
    // Get the files list
    const auto& files = vfs.get_files();
    
    // Check if all files are in the list
    EXPECT_EQ(files.size(), 3);
    EXPECT_TRUE(files.find("file1.txt") != files.end());
    EXPECT_TRUE(files.find("file2.txt") != files.end());
    EXPECT_TRUE(files.find("subdir/file3.txt") != files.end());
}

TEST_F(VFSTest, PersistenceDirCreation) {
    // Create a deeply nested persistence directory that doesn't exist
    std::string deep_dir = test_dir + "/deep/nested/dir";
    
    // This should create all necessary directories
    VirtualFileSystem vfs(deep_dir);
    
    // Verify directories were created
    EXPECT_TRUE(std::filesystem::exists(deep_dir));
    
    // Add and save a file
    std::string content = "Test content";
    std::vector<unsigned char> data = createTestData(content);
    
    vfs.add_file("test.txt", data.data(), data.size());
    EXPECT_TRUE(vfs.save_to_disk());
    
    // Verify file was saved
    EXPECT_TRUE(std::filesystem::exists(deep_dir + "/test.txt"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}