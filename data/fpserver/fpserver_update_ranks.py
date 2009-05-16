#!/usr/bin/python
import math
import sqlite3

def rankToStr(r):
    if r < 30: return "%dk" % (30-r)
    return "%dd" % (r-29)

def showRanks(connection):
    cursorUsers = connection.cursor()
    cursorUsers.execute('SELECT login, rank FROM users')
    for row in cursorUsers:
        print 'Login:%s, rank:%s' % (row[0], rankToStr(row[1]))
    cursorUsers.close()

def updateRanks(connection):
    # Config
    a = 1.0
    try:
        print ">>> Load player ranks"
        cursorUsers = connection.cursor()
        cursorUsers.execute('SELECT login FROM users')
        rank = {}
        ngames = {}
        for row in cursorUsers:
            rank[row[0]] = 10.0
            ngames[row[0]] = 0
        cursorUsers.close()

        print ">>> Process all games"
        cursorGames = connection.cursor()
        cursorGames.execute('UPDATE users SET rank=10')
        cursorGames.execute('SELECT login1, login2, rank1, rank2, winner FROM games ORDER BY date ASC')
        for game in cursorGames:
            login1, login2, rank1, rank2, winner = game
            ngames1 = ngames[login1]
            ngames2 = ngames[login2]
            ngames[login1] = ngames1 + 1
            ngames[login2] = ngames2 + 1
            # On calcule le score reel obtenu pour chaque joueur
            if winner == 1:
                S1 = 1.0
                S2 = 0.0
            else:
                S1 = 0.0
                S2 = 1.0
            # Et la proba que la partie soit gagnre pour chaque joueur
            # pour le plus fort, la formule est:
            #
            # Se(A) = 1 / (e^(D/a) + 1
            #     * D is the rating difference
            #     * a is varied depending on the prior rating of player A. (???)
            if rank1 > rank2:
                D = rank1 - rank2
                Se1 = 1.0 / (math.exp(D/a) + 1.0)
                Se2 = 1.0 - Se1
            else:
                D = rank2 - rank1
                Se2 = 1.0 / (math.exp(D/a) + 1.0)
                Se1 = 1.0 - Se2

            if ngames1 > 20:
                K1 = 0.2
            elif ngames1 > 10:
                K1 = 0.5
            else:
                K1 = 1.0
            if ngames2 > 20:
                K2 = 0.2
            elif ngames2 > 10:
                K2 = 0.5
            else:
                K2 = 1.0

            # The new rating of a player is calculated as
            # Rn = Ro + K(S - Se)
            #     * Rn = new rating
            #     * Ro = old rating
            #     * S = score (1, 0.5 or 0)
            #     * Se = expected score
            #     * K is varied depending on the rating of the players
            Rn1 = rank[login1] + K1 * (S1 - Se1)
            Rn2 = rank[login2] + K2 * (S2 - Se2)
            rank[login1] = Rn1
            rank[login2] = Rn2
        cursorGames.close()

        print ">>> Store new ranks"
        cursorUsers = connection.cursor()
        for login in rank.keys():
            # print "Rank[%s] = %f" % (login, rank[login])
            Rn = int(math.ceil(rank[login]))
            if Rn < 0: Rn = 0
            if Rn > 38: Rn = 38
            cursorUsers.execute("UPDATE users SET rank=? WHERE login=?", (Rn, login))
        cursorUsers.close()

        print ">>> Commit"
        connection.commit()
    except:
        connection.rollback()

connection = sqlite3.connect('data/fpserver/fpserver.db')
updateRanks(connection)
# showRanks(connection)
