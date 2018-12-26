# KVDB

### Software Engineering Course Project

## KV Engine

###  How to build?

```
$ cd ./engine/build
$ cmake .
$ make
```

### How to use?

```
$ ./main
```

### How to access to KVDB by java?

1. specify the location of java

```
$ cd ./engine/release
$ vim CMakeLists.txt
include_directories("/xxx/jdk1.8.0_181/include")
include_directories("/xxx/jdk1.8.0_181/include/linux")
```

2. compile

```
$ cmake .
$ make
```

3. copy the ./engine/release/libkvdb.so to /lib

```
$ sudo cp libkvdb.so /lib
```

4. import the ./javaDriver/kvdb.jar to your JAVA project, you can refer to ./javaDriver/sample.java

### HOW to add KVDB to YCSB

1. be sure that the libkvdb.so is in /lib

2. put directory ./YCSB/kvdb in the root directory of YCSB like other database.

```
$ cd ./YCSB
$ cp -r kvdb /xxx/YCSB
```
   
3. add the javaDriver to Maven local repository

```
mvn install:install-file -Dfile= /xxx/YCSB/kvdb/libs/kvdb.jar -DgroupId=com.kvdb -DartifactId=driver -Dversion=1.0.0 -Dpackaging=jar
```

4. add something in the four files: 
   
/xxx/YCSB/pom.xml
```
<module>kvdb</module>
```

/xxx/YCSB/bin/ycsb
```
"kvdb"         : "com.yahoo.ycsb.db.KVDBClient",
```

/xxx/YCSB/bin/bindings.properties

```
kvdb:com.yahoo.ycsb.db.KVDBClient
```

/xxx/YCSB/distribution/pom.xml
```
<dependency>
    <groupId>com.yahoo.ycsb</groupId>
    <artifactId>kvdb-binding</artifactId>
    <version>${project.version}</version>
</dependency>
```

5. compile
```
$ cd /xxx/YCSB
$ mvn clean package
```

6. test (you can specify the location of log and data, or the default directories are /tmp/kvdb/log; /tmp/kvdb/data)
```
$ cd /xxx/YCSB
$ ./bin/ycsb load kvdb -s -P workloads/workloada -p "logDir=/xxx/..." -p "dataDir=/xxx/..."
$ ./bin/ycsb run kvdb -s -P workloads/workloada -p "logDir=/xxx/..." -p "dataDir=/xxx/..."
```
