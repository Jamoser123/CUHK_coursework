import java.io.IOException;
import javax.naming.Context;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.Counter;
import org.apache.hadoop.mapreduce.Counters;

public class PRPreProcess {

  public static class PRPreMapper
    extends Mapper<Object, Text, Text, Text>{

    private static Text blank = new Text("");

    public void map(Object key, Text value, Context context
          ) throws IOException, InterruptedException 
    {
      String[] tokens = value.toString().split(" "); 
      Text u = new Text(tokens[0]);
      Text v = new Text(tokens[1]);

      context.write(u, v);
      context.write(v, blank);
    }
  }

  public static class PRPreReducer
    extends Reducer<Text, Text, Text, Text> {

    private static final String COUNTER_GROUP = "Custom";
    private static final String KEYWORD_COUNTER = "NodeCount";
    
    private static Text blank = new Text("");
    private long nodeCounter = 0;

    public void reduce(Text key, Iterable<Text> values,
            Context context
            ) throws IOException, InterruptedException {
      PRNodeWritable curr = new PRNodeWritable(Integer.parseInt(key.toString()));
      nodeCounter++;
      for(Text value : values){
        if(value.getLength() > 0){
          curr.appendAL(Integer.parseInt(value.toString()));
        }
      }
      curr.setPR(1.0);
      context.write(new Text(curr.toString()), blank);
    }

    @Override
    protected void cleanup(Context context) throws IOException, InterruptedException {
      context.getCounter(COUNTER_GROUP, KEYWORD_COUNTER).increment(nodeCounter);
    }
  }
}
