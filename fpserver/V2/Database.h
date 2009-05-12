#ifndef PUYO_SERVERV2_DATABASE_H
#define PUYO_SERVERV2_DATABASE_H

namespace flobopuyo {
namespace server {
namespace v2 {

class Database {
public:
    Database() {
        int rc = sqlite3_open("data/fpserver.db", &mDB);
        if (rc) {
            std::string err = sqlite3_errmsg(mDB);
            sqlite3_close(mDB);
            mDB = NULL;
            throw ios_fc::Exception((std::string("Can't open database data/base/sqlite.db: ") + err).c_str());
        }
        else {
            printf("SQLite database opened.\n");
        }
    }

    void checkLogin(const std::string &user, const std::string &password, bool &userExists, bool &passwordCorrect) {
        std::stringstream s; s << "select password from users where login='" << user << "'";
        request(s.str().c_str());
        if (vcol_head.size() > 0) {
            userExists = true;
            passwordCorrect = (password == vdata[0]); // TODO: Don't store password so clearly
        }
        else {
            userExists = false;
            passwordCorrect = false;
        }
    }

    ~Database() {
        if (mDB != NULL) {
            sqlite3_close(mDB);
            mDB = NULL;
        }
    }
private:
    sqlite3 *mDB;
    std::vector<std::string> vcol_head;
    std::vector<std::string> vdata;

    int request(const std::string &req) {
        char *zErrMsg;
        char **result;
        int nrow,ncol;
        int rc = sqlite3_get_table(mDB, req.c_str(), &result, &nrow, &ncol, &zErrMsg);

        if (vcol_head.size() !=0 ) { vcol_head.clear();  }
        if (vdata.size() != 0)     { vdata.clear(); }

        if (rc != SQLITE_OK) {
            std::stringstream error; error << "SQL error:" << zErrMsg;
            if (zErrMsg != (char*)NULL)
                    free((void*)zErrMsg);
            throw ios_fc::Exception(error.str().c_str());
        }

        if( rc == SQLITE_OK ){
            for(int i=0; i < ncol; ++i)
                vcol_head.push_back(result[i]);   /* First row heading */
            for(int i=0; i < ncol*nrow; ++i)
                vdata.push_back(result[ncol+i]);
        }
        sqlite3_free_table(result);
        return rc;
    }
};

}}}

#endif
