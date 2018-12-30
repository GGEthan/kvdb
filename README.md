### HOW to add KVDB to YCSB

1. put directory kvdb in the root directory of YCSB like other database.

```
$ cp -r kvdb /xxx/YCSB
```

2. add something in the four files: 
   
/xxx/YCSB/pom.xml;
```
<module>kvdb</module>
```

/xxx/YCSB/bin/ycsb;
```
"kvdb"         : "com.yahoo.ycsb.db.kvdb",
```

/xxx/YCSB/bindings.properties

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

3. 
```
$ cd /xxx/YCSB
$ maven clean package
```
