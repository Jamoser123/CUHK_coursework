  /**
   * MyDedup
   * 
   * TODO:
   * - test params, 32, 2048, 4096, 257
   */
  import java.io.File; 
  import java.io.FileInputStream;
  import java.io.FileOutputStream;
  import java.io.IOException;
  import java.nio.ByteBuffer;
  import java.util.LinkedList;
  import java.util.TreeMap;
  import java.util.Map.Entry;
  import java.security.MessageDigest;
  import java.security.NoSuchAlgorithmException;

  public class MyDedup {

    private int w;
    private int q;
    private int d;
    private int max;
    private int mask;

    private FileInputStream file;
    private MessageDigest md;
    private IndexStructure index;
    private FileRecipe fr;
    private Container container;
    private int uniqueChunkCount;
    private int chunkCount;
    private int byteCount;
    private int uniqueByteCount;
    private byte[] currCheckSum;
    private String currContainerName;
    private FileInputStream currCInputStream;
    private int cCFilePos;
    private ChunkMeta defaultPos = new ChunkMeta(0, 0, 0);

    public MyDedup(){
      File mydedup = new File("./mydedup.index");
      try{
        if(mydedup.exists()){
          FileInputStream indexFile = new FileInputStream(mydedup);
          this.index = new IndexStructure(indexFile);
          indexFile.close();
        }
        else{
          this.index = new IndexStructure();
        }
      }
      catch(IOException e){
        System.out.println("Error while loading index file");
        e.printStackTrace();
      }

      File dataDir = new File("./data");
      if(!dataDir.exists()){
        dataDir.mkdir();
      }
      else if(!dataDir.isDirectory()){
        dataDir.delete();
        dataDir.mkdir();
      }
    }

    public void initializeUpload(int w, int q, int d, int max, String filename) throws IOException{
      this.w = w;
      this.q = q;
      this.d = d;
      this.max = max;
      this.mask = this.q-1;

      this.file = new FileInputStream(new File(filename));
      this.fr = new FileRecipe(filename);
      this.container = new Container(this.index.containerCount);
      this.index.files.add(this.fr);
      try{
        this.md = MessageDigest.getInstance("SHA-1");
      }
      catch(NoSuchAlgorithmException e){
        System.out.println("SHA Initizalization failed");
        e.printStackTrace();
      }
    }

    public void initializeDownload(String filename) throws IOException{
      this.fr = this.index.getFR(filename);
      this.currContainerName = "";
      this.currCInputStream = new FileInputStream(new File("./mydedup.index"));
      this.cCFilePos = 0;
      if(this.fr == null){
        System.out.println("FILE NOT FOUND: " + filename);
        System.out.println("FILES IN SYSTEM");
        for(FileRecipe entry : this.index.files){
          System.out.println(entry.filename);
        }
        throw(new IOException());
      }
    }
    
    public class ChunkMeta{
      private int containerID;
      private int offset;
      private int size;

      public ChunkMeta(int id, int off, int s){
        this.containerID = id;
        this.offset = off;
        this.size = s;
      }

      public ChunkMeta(FileInputStream in) throws IOException{
        this.read(in);
      }

      public void read(FileInputStream in) throws IOException{
        byte[] intBuf = new byte[4];
        in.read(intBuf); this.containerID = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); this.offset = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); this.size = ByteBuffer.wrap(intBuf).getInt();
      }

      public void write(FileOutputStream out) throws IOException{
        out.write(ByteBuffer.allocate(4).putInt(this.containerID).array());
        out.write(ByteBuffer.allocate(4).putInt(this.offset).array());
        out.write(ByteBuffer.allocate(4).putInt(this.size).array());
      }
    }

    public class Chunk{
      byte[] data;
      int size;

      public Chunk(byte[] data, int s){
        byte[] temp = new byte[s];
        for(int i = 0; i < s; i++){
          temp[i] = data[i];
        }
        this.data = temp;
        this.size = s;
      }

      public Chunk(FileInputStream in, int size) throws IOException{
        this.read(in, size);
      }
      
      public void read(FileInputStream in, int size) throws IOException{
        byte[] dataBuf = new byte[size];
        in.read(dataBuf); this.data = dataBuf;
      }
    }

    public class Container{
      LinkedList<Chunk> chunks;
      int off;
      int size;
      int index;

      public Container(int index){
        this.chunks = new LinkedList<Chunk>();
        this.size = 0;
        this.index = index;
      }
      public int add(Chunk chunk){
        if(size + chunk.size > 1048576){
          return -1;
        }
        this.chunks.add(chunk);
        off = this.size;
        this.size += chunk.size;
        return off;
      }

      public boolean flush(){
        if(this.size == 0){
          return false;
        }
        
        try{
          File containerFile = new File("./data/container" + this.index);
          containerFile.createNewFile();
          FileOutputStream outputStream = new FileOutputStream(containerFile);
          for(Chunk chunk : this.chunks){
            outputStream.write(chunk.data);
          }
          outputStream.close();
        }
        catch(IOException e){
          System.out.println("Error occured while creating containerfile");
          e.printStackTrace();
        }      
        return true;
      }

      public void reset(){
        this.index++;
        this.size = 0;
        this.chunks.clear();
        this.off = 0;
      }
    }

    public class Fingerprint implements Comparable<Fingerprint>{
      byte[] fingerprint;

      public Fingerprint(byte[] checksum){
        this.fingerprint = checksum;
      }

      public Fingerprint(FileInputStream in) throws IOException{
        this.read(in);
      }

      @Override
      public int compareTo(Fingerprint obj){
        Integer fpValue1 = ByteBuffer.wrap(this.fingerprint).getInt();
        Integer fpValue2 = ByteBuffer.wrap(obj.fingerprint).getInt();
        return fpValue1.compareTo(fpValue2);
      }

      public void read(FileInputStream in) throws IOException{
        byte[] intBuf = new byte[4];
        in.read(intBuf); int fpLen = ByteBuffer.wrap(intBuf).getInt();
        this.fingerprint = new byte[fpLen];
        in.read(this.fingerprint);
      }

      public void write(FileOutputStream out) throws IOException{
        out.write(ByteBuffer.allocate(4).putInt(fingerprint.length).array());     
        out.write(fingerprint);
      }
    }
    
    public class FileRecipe{
      String filename;
      LinkedList<Fingerprint> recipe;

      public FileRecipe(String fn){
        this.filename = fn;
        this.recipe = new LinkedList<Fingerprint>();
      }

      public FileRecipe(FileInputStream in) throws IOException{
        this.read(in);
      }

      public void read(FileInputStream in) throws IOException{
        this.recipe = new LinkedList<Fingerprint>();
        byte[] intBuf = new byte[4];
        in.read(intBuf); int filenameLen = ByteBuffer.wrap(intBuf).getInt();

        byte[] filenameBuf = new byte[filenameLen];
        in.read(filenameBuf); this.filename = new String(filenameBuf);
        in.read(intBuf); int recipeLen = ByteBuffer.wrap(intBuf).getInt();
        for(int i = 0; i < recipeLen; i++){
          this.recipe.add(new Fingerprint(in));
        }
      }

      public void write(FileOutputStream out) throws IOException{
        byte[] filenameBytes = this.filename.getBytes();
        out.write(ByteBuffer.allocate(4).putInt(filenameBytes.length).array());
        out.write(filenameBytes);
        out.write(ByteBuffer.allocate(4).putInt(this.recipe.size()).array());
        for(Fingerprint fp : this.recipe){
          fp.write(out);
        }
      }
    }

    public class IndexStructure{
      
      private int containerCount;
      private int totalChunkCount;
      private int totalByteCount;
      private int totalUniqueChunkCount;
      private int totalUniqueByteCount;
      private LinkedList<FileRecipe> files;
      private TreeMap<Fingerprint, ChunkMeta> fpChunkMap;

      public IndexStructure(){
        this.containerCount = 0;
        this.totalChunkCount = 0;
        this.totalByteCount = 0;
        this.totalUniqueChunkCount = 0;
        this.totalUniqueByteCount = 0;
        this.files = new LinkedList<FileRecipe>(); 
        this.fpChunkMap = new TreeMap<Fingerprint, ChunkMeta>();
      }

      public IndexStructure(FileInputStream in) throws IOException{
        this.files = new LinkedList<FileRecipe>();
        this.fpChunkMap = new TreeMap<Fingerprint, ChunkMeta>();
        this.read(in);
      }

      public void addFR(FileRecipe fr){
        this.files.add(fr);
      }

      public FileRecipe getFR(String filename){
        for(FileRecipe f : this.files){
          if(f.filename.equals(filename)){
            return f;
          }
        }
        return null;
      }

      public ChunkMeta getChunkMeta(Fingerprint p){
        return this.fpChunkMap.getOrDefault(p, defaultPos);
      }

      public String report(){
        String report = "Total number of files that have been stored:\t" + this.files.size() + "\n";
        report += "Total number of pre-deduplicated chunks in storage:\t" + this.totalChunkCount + "\n";
        report += "Total number of unique chunks in storage:\t" + this.totalUniqueChunkCount + "\n";
        report += "Total number of bytes of pre-deduplicated chunks in storage:\t" + this.totalByteCount + "\n";
        report += "Total number of bytes of unique chunks in storage:\t" + this.totalUniqueByteCount + "\n";
        report += "Total number of containers in storage:\t" + this.containerCount + "\n";
        report += "Deduplication ratio:\t" + (((double) this.totalByteCount)/this.totalUniqueByteCount) + "\n";
        return report;
      }

      public void read(FileInputStream in) throws IOException{
        byte[] intBuf = new byte[4];
        in.read(intBuf); this.containerCount = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); this.totalChunkCount = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); this.totalByteCount = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); this.totalUniqueChunkCount = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); this.totalUniqueByteCount = ByteBuffer.wrap(intBuf).getInt();
        in.read(intBuf); int filesNum = ByteBuffer.wrap(intBuf).getInt();
        for(int i = 0; i < filesNum; i++){
          this.files.add(new FileRecipe(in));
        }
        in.read(intBuf); int fpChunkNum = ByteBuffer.wrap(intBuf).getInt();
        for(int i = 0; i < fpChunkNum; i++){
          this.fpChunkMap.put(new Fingerprint(in), new ChunkMeta(in));
        }
      }

      public void write(FileOutputStream out) throws IOException{
        out.write(ByteBuffer.allocate(4).putInt(this.containerCount).array());
        out.write(ByteBuffer.allocate(4).putInt(this.totalChunkCount).array());
        out.write(ByteBuffer.allocate(4).putInt(this.totalByteCount).array());
        out.write(ByteBuffer.allocate(4).putInt(this.totalUniqueChunkCount).array());
        out.write(ByteBuffer.allocate(4).putInt(this.totalUniqueByteCount).array());
        out.write(ByteBuffer.allocate(4).putInt(files.size()).array());
        for(FileRecipe recipe : files){
          recipe.write(out);
        }
        out.write(ByteBuffer.allocate(4).putInt(fpChunkMap.entrySet().size()).array());
        for(Entry<Fingerprint, ChunkMeta> entry : fpChunkMap.entrySet()){
          entry.getKey().write(out);
          entry.getValue().write(out);
        }
      }
    }

    // return n mod q, q power of 2
    public static long mod_q(int n, int q){
      return n & (q-1);
    }

    // return n mod q, q power of 2
    public static long mod_q(long n, int q){
        return n & (q-1);
    }

    public static long modular_expo(long n, int p, int q){
      long prod = 1;
      for(int j=0 ; j<p; j+=1){
          prod = (prod * (n & (q-1)) & (q-1));
      }
      return prod;
    }

    public long calcRabinInitial(int[] sub){ 
      long res = 0;
      for(int i = 0; i < this.w; i++){
        res = mod_q(res * this.d, this.q);
        res = mod_q(res + mod_q(sub[i], this.q), this.q);
      }
      return res;
    }

    public long calcRabin(long p, int t, int new_t){
      long x = modular_expo(this.d, this.w - 1, this.q);
      long new_p = mod_q(x * mod_q(t, this.q), this.q);
      new_p = mod_q(mod_q(mod_q(p - new_p, this.q) * mod_q(this.d, this.q), this.q) + new_t, this.q);
      return mod_q(new_p, this.q);
    }

    public Chunk getNextChunk() throws IOException{
      long p = 0;
      int size = 0;
      byte[] data = new byte[this.max];
      int[] window = new int[this.w];
      int startB;
      int currB = 0;

      while(size < this.w){
        currB = this.file.read();
        if(currB == -1) break;
        window[size] = currB;
        data[size] = (byte) currB;
        size++;
      }

      if(size == 0){
        return null;
      }
      else if(currB == -1){
        return new Chunk(data, size);
      }
      
      startB = window[0];
      p = this.calcRabinInitial(window);

      //check if we found an anchor point, no more data or max size reached
      while((p & this.mask) != 0 && size < this.max){
        currB = this.file.read();
        if(currB == -1) break;
        
        p = this.calcRabin(p, startB, currB);
        for(int i = 0; i < this.w - 1; i++){
          window[i] = window[i+1];
        }
        window[this.w-1] = currB;
        startB = window[0];
        data[size] = (byte) currB;
        size++;
      }

      return new Chunk(data, size);
    }

    public void incrByteCount(int bytes){
      this.byteCount += bytes;
      this.chunkCount++;
    }

    public void incrUniqueByteCount(int bytes){
      this.uniqueByteCount += bytes;
      this.uniqueChunkCount++;
    }

    public boolean uniqueChunk(Chunk chunk){
      this.md.update(chunk.data, 0, chunk.size);
      this.currCheckSum = md.digest();
      md.reset();
      ChunkMeta currPos = this.index.fpChunkMap.getOrDefault(new Fingerprint(this.currCheckSum), this.defaultPos);
      if(!currPos.equals(this.defaultPos)){
        return false;
      }
      return true;
    }

    public int addToContainer(Chunk chunk){
      return this.container.add(chunk);
    }

    public void flushContainer(){
      if(this.container.flush()){
        this.index.containerCount++;
        this.container.reset();
      }
    }

    public void updateIndex(){
      this.index.totalChunkCount += this.chunkCount;
      this.index.totalUniqueChunkCount += this.uniqueChunkCount;
      this.index.totalByteCount += this.byteCount;
      this.index.totalUniqueByteCount += this.uniqueByteCount;
    }

    public void addChunkToList(int offset, int size){
      Fingerprint newChunk = new Fingerprint(currCheckSum);
      ChunkMeta newPos = new ChunkMeta(this.container.index, offset, size);
      this.index.fpChunkMap.put(newChunk, newPos);
    }

    public void addChunkToFR(){
      this.fr.recipe.add(new Fingerprint(this.currCheckSum));
    }

    public void reportStatistics(){
      String report = "Report Output:\n";
      report += this.index.report();
      System.out.println(report);
    }

    public void writeIndex() throws IOException{
      File mydedup = new File("./mydedup.index");
      mydedup.delete();
      mydedup.createNewFile();
      FileOutputStream mydedupFile = new FileOutputStream(mydedup);
      this.index.write(mydedupFile);
      mydedupFile.close();
    }

    public void cleanup() throws IOException{
      this.file.close();
      this.writeIndex();
    }

    public Chunk getChunkFromContainer(Fingerprint p) throws IOException{
      ChunkMeta chunkPos = this.index.getChunkMeta(p);
      int containerId = chunkPos.containerID;
      int offset = chunkPos.offset;
      int size = chunkPos.size;
      if(this.currContainerName.equals("./data/container" + containerId)){
        this.currCInputStream.skip(offset-this.cCFilePos);
        this.cCFilePos = offset + size;
      }
      else{
        this.currContainerName = "./data/container" + containerId;
        this.currCInputStream = new FileInputStream(new File(this.currContainerName));
        this.cCFilePos = offset + size;
        this.currCInputStream.skip(offset);
      }
      return new Chunk(this.currCInputStream, size);
    }

    public static boolean isPowerOfTwo(int x){
      return x != 0 && ((x & (x - 1)) == 0);
    }

    public static void main(String[] args) {
      if(args[0].equals("upload")){
        int min_chunk = Integer.parseInt(args[1]);
        int avg_chunk = Integer.parseInt(args[2]);
        int max_chunk = Integer.parseInt(args[3]);
        int d = Integer.parseInt(args[4]);
        if(!(isPowerOfTwo(min_chunk) && isPowerOfTwo(avg_chunk) && isPowerOfTwo(max_chunk))){
          System.out.println("Some input params are not a power of 2! Please adjust and try to upload again.");
          return;
        }
        String uploadFile = args[5];
        
        MyDedup job = new MyDedup();
        try{
          job.initializeUpload(min_chunk, avg_chunk, d, max_chunk, uploadFile);

          Chunk chunk = job.getNextChunk();
          int offset;

          while(chunk != null){
            job.incrByteCount(chunk.size);

            if(job.uniqueChunk(chunk)){
              job.incrUniqueByteCount(chunk.size);
              offset = job.addToContainer(chunk);

              if(offset == -1){
                job.flushContainer();
                offset = job.addToContainer(chunk);
              }
              job.addChunkToList(offset, chunk.size);
            }
            job.addChunkToFR();
            chunk = job.getNextChunk();
          }
          job.flushContainer();

          job.updateIndex();

          job.reportStatistics();

          job.cleanup();
        }
        catch(IOException e){
          System.out.println("Error occured in upload");
          e.printStackTrace();
          return;
        }
      }
      else if(args[0].equals("download")){
        String dFilename = args[1];
        File localFile = new File(args[2]);

        MyDedup job = new MyDedup();
        try{
          job.initializeDownload(dFilename);
        }
        catch(IOException e){
          e.printStackTrace();
          return;
        }

        LinkedList<Chunk> chunks = new LinkedList<Chunk>();
        try{
          for(Fingerprint fp : job.fr.recipe){
            chunks.add(job.getChunkFromContainer(fp));
          }
        }
        catch(IOException e){
          System.out.println("Error reading chunks");
          e.printStackTrace();
          return;
        }

        if(localFile.exists()){
          localFile.delete();
        }
        try{
          localFile.createNewFile();
          FileOutputStream localFileOut = new FileOutputStream(localFile);
          for(Chunk chunk : chunks){
            localFileOut.write(chunk.data);
          }
          localFileOut.close();
          job.writeIndex();
        }
        catch(IOException e){
          System.out.println("Error while creating localfile");
          e.printStackTrace();
          return;
        }
      }
      else{
        System.out.println("This programm only supports upload and download.");
      }
    }
  }