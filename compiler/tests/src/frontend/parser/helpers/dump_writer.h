#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <gtest/gtest.h>

namespace valuascript::compiler::test
{
    class DumpWriter
    {
    public:
        explicit DumpWriter(const std::string& filename, const std::string& folder = "fuzz_dumps")
        {
            try
            {
                auto dump_dir = std::filesystem::current_path() / folder;
                std::filesystem::create_directories(dump_dir);
                full_path_ = dump_dir / filename;

                stream_.open(full_path_);

                if (!stream_.is_open())
                {
                    ADD_FAILURE() << "Failed to open dump file for writing: " << full_path_;
                }
            }
            catch (const std::exception& e)
            {
                ADD_FAILURE() << "Filesystem error in DumpWriter: " << e.what();
            }
        }

        std::ofstream& out() { return stream_; }

        std::string path_string() const { return full_path_.string(); }

        bool is_open() const { return stream_.is_open(); }

        ~DumpWriter()
        {
            if (stream_.is_open())
            {
                stream_.close();
            }
        }

    private:
        std::filesystem::path full_path_;
        std::ofstream stream_;
    };
}
