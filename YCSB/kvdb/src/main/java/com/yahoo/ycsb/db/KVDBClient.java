package com.yahoo.ycsb.db;

import com.yahoo.ycsb.ByteIterator;
import com.yahoo.ycsb.DB;
import com.yahoo.ycsb.DBException;
import com.yahoo.ycsb.Status;
import com.yahoo.ycsb.StringByteIterator;

import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.text.MessageFormat;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.Vector;
import org.apache.log4j.Logger;

import org.codehaus.jackson.JsonFactory;
import org.codehaus.jackson.JsonGenerator;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.ObjectNode;

import com.kvdb.Access;

/**
 * Concrete KVDB client implementaion.
 */
public class KVDBClient extends DB {

  static {
    System.loadLibrary("kvdb");
  }
  
  public static final String LOG_DIR = "logDir";
  public static final String DATA_DIR = "dataDir";
  public static final String LOG_DIR_DEFAULT = "/tmp/kvdb/log";
  public static final String DATA_DIR_DEFAULT = "/tmp/kvdb/data";

  private final Logger logger = Logger.getLogger(getClass());
  protected static final ObjectMapper MAPPER = new ObjectMapper();
  private Access kve;
  @Override
  public void init() throws DBException {
    kve = new Access();

    String logDir = getProperties.getProperty(LOG_DIR, LOG_DIR_DEFAULT);
    String dataDir = getProperties().getProperty(DATA_DIR, DATA_DIR_DEFAULT);
    kve.open(logDir, dataDir);
  }
  @Override
  public Status read(
      String table, String key, Set<String> fields,
      Map<String, ByteIterator> result) {
    key = createQualifiedKey(table, key);
    try {
      String ret = kve.read(key);
      if (null != ret) {
        fromJson(ret, fields, result);
      }
      return Status.OK;
    } catch (Exception e) {
      logger.error("Error encountered for key: " + key, e);
      return Status.ERROR;
    }
  }

  @Override
  public Status scan(
      String table, String startkey, int recordcount, Set<String> fields,
      Vector<HashMap<String, ByteIterator>> result){
    return Status.NOT_IMPLEMENTED;
  }

  @Override
  public Status update(
      String table, String key, Map<String, ByteIterator> values) {
    key = createQualifiedKey(table, key);
    try {
      boolean ret = kve.update(key, toJson(values));
      if(ret) {
        return Status.OK;
      } else { 
        return Status.ERROR;
      }
    } catch (Exception e) {
      logger.error("Error updating value with key: " + key, e);
      return Status.ERROR;
    }
  }

  @Override
  public Status insert(
      String table, String key, Map<String, ByteIterator> values) {
    key = createQualifiedKey(table, key);
    try {
      boolean ret = kve.write(key, toJson(values));
      if(ret) {
        return Status.OK;
      } else {
        return Status.ERROR;
      }
    } catch (Exception e) {
      logger.error("Error inserting value", e);
      return Status.ERROR;
    }
  }

  @Override
  public Status delete(String table, String key) {
    key = createQualifiedKey(table, key);
    try {
      boolean ret = kve.remove(key);
      if(ret) {
        return Status.OK;
      } else {
        return Status.ERROR;
      }
    } catch (Exception e) {
      logger.error("Error deleting value", e);
      return Status.ERROR;
    }
  }

  protected static void fromJson(
      String value, Set<String> fields,
      Map<String, ByteIterator> result) throws IOException {
    JsonNode json = MAPPER.readTree(value);
    boolean checkFields = fields != null && !fields.isEmpty(); //fields不为null且非空
    for (Iterator<Map.Entry<String, JsonNode>> jsonFields = json.getFields();
         jsonFields.hasNext();
         /* increment in loop body */) {
      Map.Entry<String, JsonNode> jsonField = jsonFields.next();
      String name = jsonField.getKey();
      if (checkFields && !fields.contains(name)) {
        continue;
      }
      JsonNode jsonValue = jsonField.getValue();
      if (jsonValue != null && !jsonValue.isNull()) { //两个null判断？？？
        result.put(name, new StringByteIterator(jsonValue.asText()));
      }
    }
  }

  protected static String toJson(Map<String, ByteIterator> values)
      throws IOException {
    ObjectNode node = MAPPER.createObjectNode();
    Map<String, String> stringMap = StringByteIterator.getStringMap(values);
    for (Map.Entry<String, String> pair : stringMap.entrySet()) {
      node.put(pair.getKey(), pair.getValue());
    }
    JsonFactory jsonFactory = new JsonFactory();
    Writer writer = new StringWriter();
    JsonGenerator jsonGenerator = jsonFactory.createJsonGenerator(writer);
    MAPPER.writeTree(jsonGenerator, node);
    return writer.toString();
  }

  protected static String createQualifiedKey(String table, String key) {
    return MessageFormat.format("{0}-{1}", table, key);  //"table-key"
  }
}
