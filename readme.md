# KVDB

### Software Engineering Course Project

## KV Engine

### How to build?

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

1. put directory ./YCSB/kvdb in the root directory of YCSB like other database.

```
$ cd ./YCSB
$ cp -r kvdb /xxx/YCSB
```

2. set the diretory of data and log for KVDB in kvdb/src/main/java/com/yahoo/ycsb/db/KVDBClient.java
```
  @Override
  public void init() throws DBException {
    kve = new Access();
    String logDir = "/xxx/log";
    String dataDir = "/xxx/data";
    kve.open(logDir, dataDir);
  }
```
   

3. add something in the four files: 
   
/xxx/YCSB/pom.xml;
```
<module>kvdb</module>
```

/xxx/YCSB/bin/ycsb;
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

4. compile
```
$ cd /xxx/YCSB
$ mvn clean package
```

5. test
```
$ cd /xxx/YCSB
$ ./bin/ycsb load kvdb -s -P workloads/workloada
$ ./bin/ycsb run kvdb -s -P workloads/workloada
```
