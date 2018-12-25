import com.kvdb.Access;

public class KVDB {
	static {
		System.loadLibrary("kvdb");
	}
	
	public static void main(String[] argv) {
		Access a = new Access();
		String logDir = "xxx";
		String dataDir = "xxx";
		String key = "abc";
		String value1 = "123";
		String value2 = "456";
		a.open(logDir, dataDir);
		boolean ret1 = a.write(key, value1);
		String ret2 = a.read(key);
		boolean ret3 = a.update(key, value2);
		String ret4 = a.read(key);
		boolean ret5 = a.remove(key);
		String ret6 = a.read(key);
		System.out.println("1: "+ret1);
		System.out.println("2: "+ret2);
		System.out.println("3: "+ret3);
		System.out.println("4: "+ret4);
		System.out.println("5: "+ret5);
		System.out.println("6: "+ret6);
	}
}

