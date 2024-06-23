import java.io.IOException;
import javax.naming.Context;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.mapreduce.Counter;
import org.apache.hadoop.mapreduce.Counters;

public class PageRank {

  private static final String COUNTER_GROUP = "Custom";
  private static final String KEYWORD_DANGLING = "Dangling";
  private static final String KEYWORD_NODECOUNT = "NodeCount";

  public static class PageRankMapperInitial 
    extends Mapper<Object, Text, Text, PRNodeWritable>{

      private long nodeCount;
      
      @Override 
      protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
        this.nodeCount = Long.parseLong(conf.get("nodeCount"));
      }

      public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
        PRNodeWritable curr = PRNodeWritable.fromString(value.toString());
        Text nodeID = new Text(curr.getNodeID());
        curr.setPR(1.0/this.nodeCount);
        context.write(nodeID, curr);
      }
    }

  public static class PageRankMapper
    extends Mapper<Object, Text, Text, PRNodeWritable>{
    
    private static Text dang = new Text("DANGLING");
    private PRNodeWritable sharedPR;

    public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
      PRNodeWritable curr = PRNodeWritable.fromString(value.toString());
      Text nodeID = new Text(curr.getNodeID());

      context.write(nodeID, curr);
      if(curr.getAL().size() != 0){
        for(Integer neighbor : curr.getAL()){
          sharedPR = new PRNodeWritable(0);
          sharedPR.setPR(curr.getPR()/curr.getAL().size());            
          Text neighborID = new Text(String.valueOf(neighbor));
          context.write(neighborID, sharedPR);
        }
      }
      else{
        context.write(dang, curr);
      }
    }
  }

  public static class PageRankReducerInitial 
    extends Reducer<Text,PRNodeWritable,Text,Text> {

    private static Text blank = new Text("");

    public void reduce(Text key, Iterable<PRNodeWritable> values, Context context) throws IOException, InterruptedException {
      PRNodeWritable curr = new PRNodeWritable(Integer.parseInt(key.toString()));
      for(PRNodeWritable value : values){
        if (value.getNodeID().equals(key.toString())){
          curr.setAL(value.getAL());
          curr.setPR(value.getPR());
        }
      }
      context.write(new Text(curr.toString()), blank);
    }
  }

  public static class PageRankReducer
    extends Reducer<Text,PRNodeWritable,Text,Text> {
      
    private static Text blank = new Text("");

    public void reduce(Text key, Iterable<PRNodeWritable> values, Context context) throws IOException, InterruptedException {
      if (key.toString().equals("DANGLING")){
        double danglingCounter = 0.0;
        for(PRNodeWritable value : values){
          danglingCounter += value.getPR();
        }
        context.getCounter(COUNTER_GROUP, KEYWORD_DANGLING).increment(Double.doubleToLongBits(danglingCounter));
      }
      else{
        PRNodeWritable curr = new PRNodeWritable(Integer.parseInt(key.toString()));
        double summedPR = 0;
        for(PRNodeWritable value : values){
          if (value.getNodeID().equals(key.toString())){
            curr.setAL(value.getAL());
          }
          else{
            summedPR += value.getPR();
          }
        }
        curr.setPR(summedPR);
        context.write(new Text(curr.toString()), blank);
      }
    }
  }

  public static void main(String[] args) throws Exception {
    
    Path in = new Path(args[3]);
    Path tempIn = new Path(/*args[3] + */"/tempIn");
    Path out = new Path(args[4]);
    Path tempOut = new Path(/*args[3] + */"/tempOut");
    long nodeCount = 0;
    String alpha = args[0];
    String thresh = args[2];
    String dangling = "0.0";
    int it = Integer.parseInt(args[1]);

    Job preJob;
    Job mainJob;
    Job adjustJob;
    Configuration preConf;
    Configuration mainConf;
    Configuration adjustConf;
    
    //make sure used paths don't not exist
    FileSystem fs = FileSystem.get(new Configuration());
    fs.delete(out, true);
    fs.delete(tempIn, true);
    fs.delete(tempOut, true);

    //pre process
    preConf = new Configuration();
    preJob = Job.getInstance(preConf, "pr pre");
    preJob.setJarByClass(PRPreProcess.class);
    preJob.setMapperClass(PRPreProcess.PRPreMapper.class);
    preJob.setReducerClass(PRPreProcess.PRPreReducer.class);
    preJob.setMapOutputKeyClass(Text.class);
    preJob.setMapOutputValueClass(Text.class);
    preJob.setOutputKeyClass(Text.class);
    preJob.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(preJob, in);
    FileOutputFormat.setOutputPath(preJob, tempIn);
    preJob.waitForCompletion(true);
    
    Counters counters = preJob.getCounters();
    Counter counter = counters.findCounter(COUNTER_GROUP, KEYWORD_NODECOUNT);
    nodeCount = counter.getValue();

    mainConf = new Configuration();
    mainConf.set("nodeCount", String.valueOf(nodeCount));
    mainJob = Job.getInstance(mainConf, "page rank");
    mainJob.setJarByClass(PageRank.class);
    mainJob.setMapperClass(PageRankMapperInitial.class);
    mainJob.setReducerClass(PageRankReducerInitial.class);
    mainJob.setMapOutputKeyClass(Text.class);
    mainJob.setMapOutputValueClass(PRNodeWritable.class);
    mainJob.setOutputKeyClass(Text.class);
    mainJob.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(mainJob, tempIn);
    FileOutputFormat.setOutputPath(mainJob, tempOut);
    mainJob.waitForCompletion(true);
    fs.delete(tempIn, true);

    for(int i = 1; i < it; i++){
      adjustConf = new Configuration();
      adjustConf.set("alpha", alpha);
      adjustConf.set("thresh", thresh);
      adjustConf.set("nodeCount", String.valueOf(nodeCount));
      adjustConf.set("dangling", dangling);
      adjustJob = Job.getInstance(adjustConf, "pr adjust");
      adjustJob.setJarByClass(PRAdjust.class);
      adjustJob.setJobName("pr adjust");
      adjustJob.setMapperClass(PRAdjust.PRAdjustMapper.class);
      adjustJob.setReducerClass(PRAdjust.PRAdjustReducer.class);
      adjustJob.setMapOutputKeyClass(Text.class);
      adjustJob.setMapOutputValueClass(PRNodeWritable.class);
      adjustJob.setOutputKeyClass(Text.class);
      adjustJob.setOutputValueClass(Text.class);
      FileInputFormat.addInputPath(adjustJob, tempOut);
      FileOutputFormat.setOutputPath(adjustJob, tempIn);
      adjustJob.waitForCompletion(true);

      fs.delete(tempOut, true);

      mainConf = new Configuration();
      mainJob = Job.getInstance(mainConf, "page rank");
      mainJob.setJarByClass(PageRank.class);
      mainJob.setMapperClass(PageRankMapper.class);
      mainJob.setReducerClass(PageRankReducer.class);
      mainJob.setMapOutputKeyClass(Text.class);
      mainJob.setMapOutputValueClass(PRNodeWritable.class);
      mainJob.setOutputKeyClass(Text.class);
      mainJob.setOutputValueClass(Text.class);
      FileInputFormat.addInputPath(mainJob, tempIn);
      FileOutputFormat.setOutputPath(mainJob, tempOut);
      mainJob.waitForCompletion(true);

      counters = mainJob.getCounters();
      counter = counters.findCounter(COUNTER_GROUP, KEYWORD_DANGLING);
      dangling = String.valueOf(Double.longBitsToDouble(counter.getValue()));

      fs.delete(tempIn, true);
    }

    //cleanup
    adjustConf = new Configuration();
    adjustConf.set("alpha", alpha);
    adjustConf.set("thresh", thresh);
    adjustConf.set("nodeCount", String.valueOf(nodeCount));
    adjustConf.set("dangling", dangling);
    adjustJob = Job.getInstance(adjustConf, "pr adjust");
    adjustJob.setJarByClass(PRAdjust.class);
    adjustJob.setJobName("pr adjust");
    adjustJob.setMapperClass(PRAdjust.PRAdjustMapper.class);
    adjustJob.setReducerClass(PRAdjust.PRAdjustReducerCleanup.class);
    adjustJob.setMapOutputKeyClass(Text.class);
    adjustJob.setMapOutputValueClass(PRNodeWritable.class);
    adjustJob.setOutputKeyClass(Text.class);
    adjustJob.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(adjustJob, tempOut);
    FileOutputFormat.setOutputPath(adjustJob, out);
    adjustJob.waitForCompletion(true);
    fs.delete(tempOut, true);
  }
}