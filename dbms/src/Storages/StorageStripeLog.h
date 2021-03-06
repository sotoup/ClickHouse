#pragma once

#include <map>

#include <ext/shared_ptr_helper.hpp>

#include <Poco/File.h>

#include <Storages/IStorage.h>
#include <Common/FileChecker.h>
#include <Common/escapeForFileName.h>


namespace DB
{

/** Implements a repository that is suitable for small pieces of the log.
  * In doing so, stores all the columns in a single Native file, with a nearby index.
  */
class StorageStripeLog : private ext::shared_ptr_helper<StorageStripeLog>, public IStorage
{
friend class ext::shared_ptr_helper<StorageStripeLog>;
friend class StripeLogBlockInputStream;
friend class StripeLogBlockOutputStream;

public:
    /** hook the table with the appropriate name, along the appropriate path (with / at the end),
      *  (the correctness of names and paths is not checked)
      *  consisting of the specified columns.
      * If not specified `attach` - create a directory if it does not exist.
      */
    static StoragePtr create(
        const std::string & path_,
        const std::string & name_,
        NamesAndTypesListPtr columns_,
        const NamesAndTypesList & materialized_columns_,
        const NamesAndTypesList & alias_columns_,
        const ColumnDefaults & column_defaults_,
        bool attach,
        size_t max_compress_block_size_ = DEFAULT_MAX_COMPRESS_BLOCK_SIZE);

    std::string getName() const override { return "StripeLog"; }
    std::string getTableName() const override { return name; }

    const NamesAndTypesList & getColumnsListImpl() const override { return *columns; }

    BlockInputStreams read(
        const Names & column_names,
        const ASTPtr & query,
        const Context & context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size = DEFAULT_BLOCK_SIZE,
        unsigned threads = 1) override;

    BlockOutputStreamPtr write(const ASTPtr & query, const Settings & settings) override;

    void rename(const String & new_path_to_db, const String & new_database_name, const String & new_table_name) override;

    bool checkData() const override;

    /// Data of the file.
    struct ColumnData
    {
        Poco::File data_file;
    };
    using Files_t = std::map<String, ColumnData>;

    std::string full_path() { return path + escapeForFileName(name) + '/';}

private:
    String path;
    String name;
    NamesAndTypesListPtr columns;

    size_t max_compress_block_size;

    FileChecker file_checker;
    Poco::RWLock rwlock;

    Logger * log;

    StorageStripeLog(
        const std::string & path_,
        const std::string & name_,
        NamesAndTypesListPtr columns_,
        const NamesAndTypesList & materialized_columns_,
        const NamesAndTypesList & alias_columns_,
        const ColumnDefaults & column_defaults_,
        bool attach,
        size_t max_compress_block_size_);
};

}
