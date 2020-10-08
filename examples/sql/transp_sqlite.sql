BEGIN;
# production capacity
DROP TABLE IF EXISTS transp_capa;
CREATE TABLE transp_capa (
  PLANT TEXT(127),
  CAPA  REAL,
  PRIMARY KEY ( PLANT )
  );
INSERT INTO transp_capa ( PLANT, CAPA ) VALUES ( 'Seattle',   350 );
INSERT INTO transp_capa ( PLANT, CAPA ) VALUES ( 'San Diego', 600 );
# demand
DROP TABLE IF EXISTS transp_demand;
CREATE TABLE transp_demand (
  MARKET TEXT(127),
  DEMAND REAL,
  PRIMARY KEY ( MARKET )
  );
INSERT INTO transp_demand ( MARKET, DEMAND ) VALUES ( 'New York', 325 );
INSERT INTO transp_demand ( MARKET, DEMAND ) VALUES ( 'Chicago', 300 );
INSERT INTO transp_demand ( MARKET, DEMAND ) VALUES ( 'Topeka', 275 );
# distance
DROP TABLE IF EXISTS transp_dist;
CREATE TABLE transp_dist (
  LOC1 TEXT(127),
  LOC2 TEXT(127),
  DIST REAL,
  PRIMARY KEY ( LOC1, LOC2 )
  );
INSERT INTO transp_dist ( LOC1, LOC2, DIST ) VALUES ( 'Seattle',   'New York', 2.5 );
INSERT INTO transp_dist ( LOC1, LOC2, DIST ) VALUES ( 'Seattle',   'Chicago', 1.7 );
INSERT INTO transp_dist ( LOC1, LOC2, DIST ) VALUES ( 'Seattle',   'Topeka', 1.8 );
INSERT INTO transp_dist ( LOC1, LOC2, DIST ) VALUES ( 'San Diego', 'New York', 2.5 );
INSERT INTO transp_dist ( LOC1, LOC2, DIST ) VALUES ( 'San Diego', 'Chicago', 1.8 );
INSERT INTO transp_dist ( LOC1, LOC2, DIST ) VALUES ( 'San Diego', 'Topeka', 1.4 );
# result
DROP TABLE IF EXISTS transp_result;
CREATE TABLE transp_result (
  LOC1     TEXT(127),
  LOC2     TEXT(127),
  QUANTITY REAL,
  PRIMARY KEY ( LOC1, LOC2 )
  );
COMMIT;
