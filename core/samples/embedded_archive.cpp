#include <cubos/core/data/embedded_archive.hpp>
#include <cubos/core/data/file_system.hpp>

int main()
{
    using namespace cubos::core;
    using namespace cubos::core::data;

    // The name passed to the constructor of the embedded archive must be the same as the name used when embedding the
    // data. In this case, since the name wasn't specified, the name of the root directory was used.
    FileSystem::mount("/", std::make_unique<EmbeddedArchive>("assets"));

    // The file system will now be able to access the embedded archive.
    std::string contents;
    auto stream = FileSystem::open("/foo.txt", File::OpenMode::Read);
    stream->readUntil(contents, nullptr);
    memory::Stream::stdOut.printf("Read \"{}\" from '/foo.txt'.\n", contents);

    stream = FileSystem::open("/bar/baz.txt", File::OpenMode::Read);
    stream->readUntil(contents, nullptr);
    memory::Stream::stdOut.printf("Read \"{}\" from '/bar/baz.txt'.\n", contents);

    FileSystem::unmount("/");

    return 0;
}

/// The content below, which would usually be placed in a separate source file, was generated by the cubos-embed tool
/// with the following arguments:
///     cubos-embed -r assets >> embedded_archive.cpp

/// This file was generated by cubos-embed.
/// Do not edit this file.
///
/// To use this file in your project, you just need to link
/// this source file by adding it to your source files. To
/// use it in an EmbeddedArchive, you just need to construct
/// an EmbeddedArchive with the data name 'assets'

#include <array>

using namespace cubos::core::data;

static const uint8_t FileData3[] = {
    0x62,
    0x61,
    0x7a,
    0xa,
};

static const uint8_t FileData4[] = {
    0x66,
    0x6f,
    0x6f,
    0xa,
};

static const EmbeddedArchive::Data::Entry Entries[] = {
    {"", true, 0, 0, 2, nullptr, 0},
    {"bar", true, 1, 4, 3, nullptr, 0},
    {"baz.txt", false, 2, 0, 0, FileData3, sizeof(FileData3)},
    {"foo.txt", false, 1, 0, 0, FileData4, sizeof(FileData4)},
};

static const EmbeddedArchive::Data EmbeddedArchiveData = {
    Entries,
    sizeof(Entries) / sizeof(Entries[0]),
};

class DataRegister
{
public:
    DataRegister()
    {
        EmbeddedArchive::registerData("assets", EmbeddedArchiveData);
    }
};

static DataRegister dataRegister;
