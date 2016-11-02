<!-- Name: MacPostGIS_Setup -->
<!-- Version: 4 -->
<!-- Last-Modified: 2008/11/10 13:47:55 -->
<!-- Author: springmeyer -->
## Mac PostGIS /  Postgres Setup

After you have installed PostgreSQL and PostGIS you will need to perform further setup before you can begin to use them with mapnik.

 * Setup for *Binaries/Frameworks* from [kyngchaos.com](http://www.kyngchaos.com/wiki/software:frameworks).
  * Add the bin and man page directories to your path (assumes bash shell which is default on 10.5).  You may create or append the ~/.profile  with the following:

```sh
    export PATH=/usr/local/pgsql/bin:$PATH
    export MANPATH=/usr/local/pgsql/man:$MANPATH
```

  * Register your changes

```sh
   $ source ~/.profile
```

You should now be able to type the following from any directory and see the man page.

```sh
    $ man psql
```

  * Create a database.  The installer you used from   [kyngchaos.com](http://www.kyngchaos.com/wiki/software:frameworks) automatically created a Mac OS user on your system named postgres.  Use that user to create a new database user/role and setup the database. (change "gisuser" to your liking) 

```sh
    $ sudo -u postgres -i
    $ createuser gisuser
    $ createdb -E UTF8 -O gisuser mapnik
    $ createlang plpgsql mapnik
```

  * Alternatively you can run all commands from your normal shell user but use the '-U' flag to call them as the postgres user:

```sh
    $ createdb -E UTF8 -U postgres <dbname>
    $ createlang plpgsql -U postgres <dbname>
```

  * PostGIS support must be enabled for each database that requires its usage. This is done by feeding the lwpostgis.sql (the enabler script) file to the target database. 

```sh
    $ psql -d mapnik -f /usr/local/pgsql/share/lwpostgis.sql
    $ psql -d mapnik -f /usr/local/pgsql/share/spatial_ref_sys.sql
    $ echo "ALTER TABLE geometry_columns OWNER TO gisuser; ALTER TABLE spatial_ref_sys OWNER TO gisuser;"  | psql -d mapnik
```

 At this point your database should be setup to create a PostGIS table.
TODO - full example showing PostGIS use.