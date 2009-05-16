#ifndef PUYO_SERVERV2_DATABASE_H
#define PUYO_SERVERV2_DATABASE_H

namespace flobopuyo {
namespace server {
namespace v2 {

class Database {
public:
    Database() {
        int rc = sqlite3_open("data/fpserver/fpserver.db", &mDB);
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

    void checkLogin(const std::string &user, const std::string &password,
                    bool &userExists, bool &userExpired, bool &passwordCorrect) {
        std::stringstream s; s << "select password,end_date from users where login='" << user << "'";
        request(s.str().c_str());
        if (vcol_head.size() > 0) {
            userExists = true;
            double end_date = atol(vdata[1].c_str());
            passwordCorrect = (password == vdata[0]); // TODO: Don't store password so clearly
            userExpired = (end_date < ios_fc::getUnixTime());
            printf("Login '%s' exists and is valid for %g days\n", user.c_str(), (end_date-ios_fc::getUnixTime())/(24.*3600.));
        }
        else {
            userExists = false;
            passwordCorrect = false;
        }
    }

    void loadPeerInfos(Peer *peer) {
        std::stringstream s; s << "select rank,end_date from users where login='" << peer->name << "'";
        request(s.str().c_str());
        if (vcol_head.size() > 0) {
            peer->rank = atoi(vdata[0].c_str());
            peer->end_date = strtol(vdata[1].c_str(), NULL, 10);
        }
    }

    void storeConnection(const std::string &user) {
        // TODO: ?
        // Log generiques ou Log des connections...
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
