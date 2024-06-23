import java.io.IOException;
import java.io.InterruptedIOException;
import javax.naming.Context;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;

public class PRAdjust {

  public static class PRAdjustMapper
    extends Mapper<Object, Text, Text, PRNodeWritable>{
    
    public void map(Object key, Text value, Context context
          ) throws IOException, InterruptedException {
      PRNodeWritable curr = PRNodeWritable.fromString(value.toString());
      Text nodeID = new Text(curr.getNodeID());
      context.write(nodeID, curr);
    }
  }

  public static class PRAdjustReducerCleanup
    extends Reducer<Text,PRNodeWritable,Text,Text> {
    
    private double alpha;
    private double thresh;
    private int nodeCount;
    private double danglingMass;

    @Override
    protected void setup(Context context) throws IOException, InterruptedException {
      Configuration conf = context.getConfiguration();
      this.alpha = Double.parseDouble(conf.get("alpha"));
      this.thresh = Double.parseDouble(conf.get("thresh"));
      this.nodeCount = Integer.parseInt(conf.get("nodeCount"));
      this.danglingMass = Double.parseDouble(conf.get("dangling"));
    }

    public void reduce(Text key, Iterable<PRNodeWritable> values, Context context)
        throws IOException, InterruptedException {

      PRNodeWritable curr = new PRNodeWritable(Integer.parseInt(key.toString()));
      for(PRNodeWritable value : values){
        if (value.getNodeID().equals(key.toString())){
          curr.setAL(value.getAL());
          curr.setPR(value.getPR());
        }
      }
      curr.setPR(alpha*(1.0/nodeCount) + (1-alpha)*((danglingMass/nodeCount) + curr.getPR()));
      if(curr.getPR() >= this.thresh){
        context.write(new Text(curr.getNodeID()), new Text(String.valueOf(curr.getPR())));
      }
    }
  }

  public static class PRAdjustReducer
    extends Reducer<Text,PRNodeWritable,Text,Text> {
      
    private double alpha;
    private double thresh;
    private int nodeCount;
    private double danglingMass;
    private static Text blank = new Text("");

    @Override
    protected void setup(Context context) throws IOException, InterruptedException {
      Configuration conf = context.getConfiguration();
      this.alpha = Double.parseDouble(conf.get("alpha"));
      this.thresh = Double.parseDouble(conf.get("thresh"));
      this.nodeCount = Integer.parseInt(conf.get("nodeCount"));
      this.danglingMass = Double.parseDouble(conf.get("dangling"));
    }

    public void reduce(Text key, Iterable<PRNodeWritable> values, Context context)
        throws IOException, InterruptedException {
      
      PRNodeWritable curr = new PRNodeWritable(Integer.parseInt(key.toString()));
      for (PRNodeWritable value : values){
        if (value.getNodeID().equals(key.toString())){
          curr.setAL(value.getAL());
          curr.setPR(value.getPR());
        }
      }
      curr.setPR(alpha*(1.0/nodeCount) + (1-alpha)*((danglingMass/nodeCount) + curr.getPR()));
      context.write(new Text(curr.toString()), blank);
    }
  }
}