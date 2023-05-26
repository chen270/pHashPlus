#include "pHash_mvp.h"
#include <fstream>
#include <string>
#include "sqlite3/sqlite3.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

float hammingdistance(DP *pntA, DP *pntB) {
    HashDataType hdataTypeA = pntA->hash_datatype;
    HashDataType hdataTypeB = pntB->hash_datatype;
    if (hdataTypeA != hdataTypeB)
        return -1.0;
    if (hdataTypeA != UINT64ARRAY)
        return -1.0;
    if ((pntA->hash_length > 1) || (pntB->hash_length > 1))
        return -1.0;
    ulong64 *hashA = (ulong64 *)pntA->hash;
    ulong64 *hashB = (ulong64 *)pntB->hash;
    int res = ph_hamming_distance(*hashA, *hashB);
    return (float)res;
}

static bool ph_file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

static std::string ph_db_table_name(const HashType hash_type) {
    std::string tableName;
    switch (hash_type) {
        case TEXT:
            tableName = "text_hashes";
            break;
        case IMAGE:
            tableName = "image_hashes";
            break;
        case AUDIO:
            tableName = "audio_hashes";
            break;
        case VIDEO:
            tableName = "video_hashes";
            break;
        default:
            fprintf(stderr, "HashType(%d) No Support\n", hash_type);
    }
    return tableName;
}

// Serialize array data to JSON strings
std::string serializeArray(const void *array, uint32_t length, HashDataType datatype) {
    json jsonData;
    switch (datatype) {
        case BYTEARRAY: {
            const uint8_t *byteArray = static_cast<const uint8_t *>(array);
            jsonData["hash"] = std::vector<uint8_t>(byteArray, byteArray + length);
            break;
        }
        case UINT16ARRAY: {
            const uint16_t *uint16Array = static_cast<const uint16_t *>(array);
            jsonData["hash"] = std::vector<uint16_t>(uint16Array, uint16Array + length);
            break;
        }
        case UINT32ARRAY: {
            const uint32_t *uint32Array = static_cast<const uint32_t *>(array);
            jsonData["hash"] = std::vector<uint32_t>(uint32Array, uint32Array + length);
            break;
        }
        case UINT64ARRAY: {
            const uint64_t *uint64Array = static_cast<const uint64_t *>(array);
            jsonData["hash"] = std::vector<uint64_t>(uint64Array, uint64Array + length);
            break;
        }
        default: {
            // Handle the case when datatype is not recognized
            fprintf(stderr, "Unsupported HashDataType(%d)\n", datatype);
            break;
        }
    }
    return jsonData.dump();
}

// Deserialize JSON strings into arrays of data
static void deserializeArray(const std::string &jsonString, void **array, uint32_t length, HashDataType datatype) {
    json jsonData = json::parse(jsonString);
    switch (datatype) {
        case BYTEARRAY: {
            std::vector<uint8_t> byteArray = jsonData["hash"].get<std::vector<uint8_t>>();
            *array = malloc(length * sizeof(uint8_t));
            std::memcpy(*array, byteArray.data(), length);
            break;
        }
        case UINT16ARRAY: {
            std::vector<uint16_t> uint16Array = jsonData["hash"].get<std::vector<uint16_t>>();
            *array = malloc(length * sizeof(uint16_t));
            std::memcpy(*array, uint16Array.data(), length * sizeof(uint16_t));
            break;
        }
        case UINT32ARRAY: {
            std::vector<uint32_t> uint32Array = jsonData["hash"].get<std::vector<uint32_t>>();
            *array = malloc(length * sizeof(uint32_t));
            std::memcpy(*array, uint32Array.data(), length * sizeof(uint32_t));
            break;
        }
        case UINT64ARRAY: {
            std::vector<uint64_t> uint64Array = jsonData["hash"].get<std::vector<uint64_t>>();
            *array = malloc(length * sizeof(uint64_t));
            std::memcpy(*array, uint64Array.data(), length * sizeof(uint64_t));
            break;
        }
        default: {
            // Handle the case when datatype is not recognized
            fprintf(stderr, "Unsupported HashDataType(%d)\n", datatype);
            break;
        }
    }
}

MVPRetCode ph_sizeof_mvptree(MVPFile *m, int64_t &result) {
    if (!m)
        return PH_ERRNULLARG;

    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);
    if (!ph_file_exists(mainSqlite)) {
        fprintf(stderr, "File %s does not exist!\n", mainSqlite);
        return PH_ERRFILEEXIST;
    }

    sqlite3 *db;
    int rc = sqlite3_open(mainSqlite, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    const char *querySql = "SELECT page_count * page_size FROM pragma_page_count(), pragma_page_size();";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, querySql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Sqlite3 Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return PH_ERRPREPARE;
    }

    MVPRetCode ret = PH_SUCCESS;
    result = 0;

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_int64(stmt, 0);
    } else {
        ret = PH_ERRDPFOUND;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return ret;
}

MVPRetCode ph_query_mvptree(MVPFile *m, const char *id, DP *result) {
    if (!m || !id) {
        return PH_ERRNULLARG;
    }

    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);
    if (!ph_file_exists(mainSqlite)) {
        fprintf(stderr, "File %s does not exist!\n", mainSqlite);
        return PH_ERRFILEEXIST;
    }

    sqlite3 *db;
    int rc = sqlite3_open(mainSqlite, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    sqlite3_stmt *stmt;
    std::string selectQuery = "SELECT * FROM " + ph_db_table_name(m->hash_type) + " WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Sqlite3 Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return PH_ERRPREPARE;
    }

    MVPRetCode ret = PH_SUCCESS;

    sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char *retrievedId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        size_t idLength = std::strlen(retrievedId);
        result->id = (char*)malloc((idLength + 1) * sizeof(char));
        std::strcpy(result->id, retrievedId);

        const char *retrievedPath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        size_t pathLength = std::strlen(retrievedPath);
        result->path = (char*)malloc((pathLength + 1) * sizeof(char));

        std::strcpy(result->path, retrievedPath);

        result->hash_length = sqlite3_column_int(stmt, 3);
        result->hash_datatype = static_cast<HashDataType>(sqlite3_column_int(stmt, 4));

        const unsigned char *retrievedHashJson = sqlite3_column_text(stmt, 1);
        std::string hashJson(reinterpret_cast<const char *>(retrievedHashJson));
        deserializeArray(hashJson, &(result->hash), result->hash_length, result->hash_datatype);
    } else {
        ret = PH_ERRDPFOUND;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ret;
}

MVPRetCode ph_query_mvptree(MVPFile *m, DP *query, float threshold, DP **results, int *count) {
    if (!m || !query || !results || !count) {
        return PH_ERRNULLARG;
    }

    if (m->hash_type != query->hash_type)
        return PH_ERRHASHTYPE;

    hash_compareCB hashdist = m->hashdist;
    if (!hashdist)
        return PH_ERRNULLARG;

    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);
    if (!ph_file_exists(mainSqlite)) {
        fprintf(stderr, "File %s does not exist!\n", mainSqlite);
        return PH_ERRFILEEXIST;
    }

    // Open the database
    sqlite3 *db;
    int rc = sqlite3_open(mainSqlite, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    // Prepare the query statement
    std::string querySql = "SELECT * FROM ";
    querySql += ph_db_table_name(m->hash_type);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Sqlite3 Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return PH_ERRPREPARE;
    }

    // Execute the query and process the results
    int resultCount = 0;
    DP **queryResults = nullptr;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Create a DP struct to hold the retrieved data
        DP *dp = (DP *)malloc(sizeof(DP));

        const unsigned char *id = sqlite3_column_text(stmt, 0);
        dp->id = strdup((const char *)id);

        const unsigned char *path = sqlite3_column_text(stmt, 2);
        dp->path = strdup((const char *)path);

        dp->hash_length = sqlite3_column_int(stmt, 3);
        dp->hash_datatype = static_cast<HashDataType>(sqlite3_column_int(stmt, 4));

        // hash code
        const unsigned char *retrievedHashJson = sqlite3_column_text(stmt, 1);
        std::string hashJson(reinterpret_cast<const char *>(retrievedHashJson));
        deserializeArray(hashJson, &dp->hash, dp->hash_length, dp->hash_datatype);

        // Compare the query DP with the retrieved DP using the comparison function
        if (hashdist(query, dp) < threshold) {
            // DP matches the query criteria, add it to the results array
            queryResults = (DP **)realloc(queryResults, (resultCount + 1) * sizeof(DP *));
            queryResults[resultCount] = dp;
            resultCount++;
        } else {
            // DP doesn't match, free the allocated memory
            free(dp->id);
            free(dp->hash);
            free(dp->path);
            free(dp);
        }
    }

    // Finalize the query statement
    sqlite3_finalize(stmt);

    // Close the database
    sqlite3_close(db);

    // Assign the results and count to the output parameters
    results = queryResults;
    *count = resultCount;

    return PH_SUCCESS;
}


MVPRetCode ph_create_mvptree(MVPFile *m) {
    if (!m) {
        return PH_ERRARG;
    }

    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);

    sqlite3 *db;
    int rc = sqlite3_open_v2(mainSqlite, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    std::string createTableQuery = "CREATE TABLE IF NOT EXISTS ";
    createTableQuery += ph_db_table_name(m->hash_type);
    createTableQuery += " (id TEXT PRIMARY KEY, hash TEXT, path TEXT, hash_length INTEGER, hash_datatype INTEGER);";

    char *errMsg = nullptr;
    rc = sqlite3_exec(db, createTableQuery.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Create table failed: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return PH_ERRTABLECREATE;
    }

    sqlite3_close(db);
    return PH_SUCCESS;
}

MVPRetCode ph_create_mvptree(MVPFile *m, DP **points, int nbpoints) {
    if ((!m) || (!points)) {
        return PH_ERRARG;
    }

    MVPRetCode ret = ph_create_mvptree(m);
    if (ret != PH_SUCCESS) {
        return ret;
    }

    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);

    sqlite3 *db;
    int rc = sqlite3_open(mainSqlite, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    // add points
    std::string insertQuery = "INSERT INTO ";
    insertQuery += ph_db_table_name(m->hash_type);
    insertQuery += " (id, hash, path, hash_length, hash_datatype) VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt *statement;
    rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &statement, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Sqlite3 Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(statement);
        sqlite3_close(db);
        return PH_ERRPREPARE;
    }

    // Bind data
    for (int i = 0; i < nbpoints; ++i) {
        DP *dp = points[i];
        if (dp->hash_type != m->hash_type) {
            fprintf(stderr, "Failed to insert data point. Input points[%d]->hash_type != MVPFile->hash_type.\n", i);
            continue;
        }

        if (dp->id == nullptr)
        {
            fprintf(stderr, "Failed to insert data point. Input points[%d]->id is NULL.\n", i);
            continue;
        }

        if (dp->hash == nullptr) {
            fprintf(stderr, "Failed to insert data point. Input points[%d]->hash is NULL.\n", i);
            continue;
        }

        sqlite3_bind_text(statement, 1, dp->id, -1, SQLITE_STATIC);

        // Serialize hash arrays to JSON strings
        std::string hashJson = serializeArray(dp->hash, dp->hash_length, dp->hash_datatype);
        sqlite3_bind_text(statement, 2, hashJson.c_str(), -1, SQLITE_STATIC);

        if (dp->path == nullptr)
            sqlite3_bind_text(statement, 3, "NO INPUT", -1, SQLITE_STATIC);
        else
            sqlite3_bind_text(statement, 3, dp->path, -1, SQLITE_STATIC);
        sqlite3_bind_int(statement, 4, dp->hash_length);
        sqlite3_bind_int(statement, 5, dp->hash_datatype);

        rc = sqlite3_step(statement);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to intser dp[%d] point: %s\n", i, sqlite3_errmsg(db));
        }

        sqlite3_reset(statement);
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return ret;
}

MVPRetCode ph_add_mvptree(MVPFile *m, DP *new_dp) {
    if ((!m) || (!new_dp) || (!new_dp->id) || (!new_dp->hash) )
        return PH_ERRNULLARG;

    if (m->hash_type != new_dp->hash_type)
        return PH_ERRHASHTYPE;

    // check file exist
    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);
    if (!ph_file_exists(mainSqlite)) {
        fprintf(stderr, "File %s does not exist!\n", mainSqlite);
        return PH_ERRFILEEXIST;
    }

    // open db
    sqlite3 *db;
    int rc = sqlite3_open(mainSqlite, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    MVPRetCode ret = PH_SUCCESS;

    // insert
    std::string insertQuery = "INSERT INTO ";
    insertQuery += ph_db_table_name(m->hash_type);
    insertQuery += " (id, hash, path, hash_length, hash_datatype) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Sqlite3 Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return PH_ERRPREPARE;
    }

    // bind
    sqlite3_bind_text(stmt, 1, new_dp->id, -1, SQLITE_STATIC);

    // Serialize hash arrays to JSON strings
    std::string hashJson = serializeArray(new_dp->hash, new_dp->hash_length, new_dp->hash_datatype);
    sqlite3_bind_text(stmt, 2, hashJson.c_str(), -1, SQLITE_STATIC);

    if (new_dp->path == nullptr)
        sqlite3_bind_text(stmt, 3, "NO INPUT", -1, SQLITE_STATIC);
    else
        sqlite3_bind_text(stmt, 3, new_dp->path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, new_dp->hash_length);
    sqlite3_bind_int(stmt, 5, new_dp->hash_datatype);

    rc = sqlite3_step(stmt);  // run
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insert data failed: %s\n", sqlite3_errmsg(db));
        ret = PH_ERRINSERT;
    }

    sqlite3_finalize(stmt);  // release
    sqlite3_close(db);       // close
    return ret;
}

MVPRetCode ph_add_mvptree(MVPFile *m, DP **points, int nbpoints, int &nbsaved) {
    if ((!m) || (!points) || nbpoints <= 0)
        return PH_ERRNULLARG;

    // check file exists
    char mainSqlite[MAX_PATH];
    snprintf(mainSqlite, sizeof(mainSqlite), "%s.db", m->filename);
    if (!ph_file_exists(mainSqlite)) {
        fprintf(stderr, "File %s does not exist!\n", mainSqlite);
        return PH_ERRFILEEXIST;
    }

    // open database
    sqlite3 *db;
    int rc = sqlite3_open(mainSqlite, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return PH_ERRFILEOPEN;
    }

    // start transaction
    rc = sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to start transaction.\n");
        sqlite3_close(db);
        return PH_ERRTABLECREATE;
    }

    // insert data points
    std::string insertQuery = "INSERT INTO ";
    insertQuery += ph_db_table_name(m->hash_type);
    insertQuery += " (id, hash, path, hash_length, hash_datatype) VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Sqlite3 Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return PH_ERRPREPARE;
    }

    // bind parameters
    nbsaved = 0;
    for (int i = 0; i < nbpoints; ++i) {
        DP *dp = points[i];
        if (dp->hash_type != m->hash_type)
        {
            fprintf(stderr, "Failed to insert data point. Input points[%d]->hash_type != MVPFile->hash_type.\n", i);
            continue;
        }

        if (dp->id == nullptr) {
            fprintf(stderr, "Failed to insert data point. Input points[%d]->id is NULL.\n", i);
            continue;
        }

        if (dp->hash == nullptr) {
            fprintf(stderr, "Failed to insert data point. Input points[%d]->hash is NULL.\n", i);
            continue;
        }

        sqlite3_bind_text(stmt, 1, dp->id, -1, SQLITE_STATIC);

        // 序列化 hash 数组为 JSON 字符串
        std::string hashJson = serializeArray(dp->hash, dp->hash_length, dp->hash_datatype);
        sqlite3_bind_text(stmt, 2, hashJson.c_str(), -1, SQLITE_STATIC);

        if (dp->path == nullptr)
            sqlite3_bind_text(stmt, 3, "NO INPUT", -1, SQLITE_STATIC);
        else
            sqlite3_bind_text(stmt, 3, dp->path, -1, SQLITE_STATIC);

        sqlite3_bind_int(stmt, 4, dp->hash_length);
        sqlite3_bind_int(stmt, 5, dp->hash_datatype);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to insert data point. Skipping.\n");
            sqlite3_reset(stmt);
            continue;
        }

        sqlite3_reset(stmt);
        ++nbsaved;
    }

    MVPRetCode ret = PH_SUCCESS;

    // commit transaction
    rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to commit transaction.\n");
        ret = PH_ERRSUBMIT;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return ret;
}
