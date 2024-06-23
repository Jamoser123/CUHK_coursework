import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

import javax.naming.Context;

import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.MapWritable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

public class NgramInitialRF {

  public static class TokenizerMapper
    extends Mapper<Object, Text, Text, MapWritable>{

    private MapWritable hMap = new MapWritable();
    private Text totalCount = new Text("*");
    private int N;
    private char[] ngram;
    private int c;

    @Override
    protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
        N = Integer.parseInt(conf.get("N"));
        ngram = new char[2*N];
        c = 2;
    }

    public void map(Object key, Text value, Context context
          ) throws IOException, InterruptedException {
      String[] tokens = value.toString().split("[^a-zA-Z]"); //Parse input
      MapWritable currM;
      IntWritable currC;

      for(String token : tokens){
        if (!token.isEmpty()){
          char initial = token.charAt(0); //initial of new word
          if (c < 2*N){ //not enough characters yet
            ngram[c] = initial;
            ngram[c-1] = ' ';
            c += 2;
          }
          else{
            if (N == 1){
              ngram[0] = initial;
            }
            else{
              for (int i = 0; i < (2*N)-2; i += 2){ //move characters one to the left and put initial of new word on the rightmost spot
              ngram[i] = ngram[i+2];
              }
              ngram[2*N-2] = initial;
            }
            Text kT = new Text(String.valueOf(ngram[0])); //inital character of ngram
            Text subkT = new Text(String.valueOf(ngram, 1, 2*N - 2)); //other characters of ngram

            if (hMap.containsKey(kT)){
              currM = (MapWritable) hMap.get(kT); //Stripe for initial character of ngram

              //update totalCount
              currC = (IntWritable) currM.get(totalCount);
              currC.set(currC.get() + 1);
              currM.put(totalCount, currC);

              //if Stripe already contains other characters update count
              if(currM.containsKey(subkT)){ 
                currC = (IntWritable) currM.get(subkT);
                currC.set(currC.get() + 1);
                currM.put(subkT, currC);
              }
              else{//else make new entry 
                currM.put(subkT, new IntWritable(1)); 
              }
              hMap.put(kT, currM);
            }
            else{ //no Stripe for initial character of ngram -> Create new one
              currM = new MapWritable();
              currM.put(subkT, new IntWritable(1));
              currM.put(totalCount, new IntWritable(1));
              hMap.put(kT, currM);
            }
          }
        }
      }
    }

    @Override
    protected void cleanup(Context context) throws IOException, InterruptedException {
      for (Map.Entry<Writable, Writable> entry : hMap.entrySet()) {
        context.write((Text) entry.getKey(), (MapWritable) entry.getValue()); //Initial of Ngram + Stripe of Initial
      }
    }
  }

  public static class IntSumReducer
    extends Reducer<Text,MapWritable,Text,DoubleWritable> {

    private Text finalNgram = new Text();
    private DoubleWritable finalRatio = new DoubleWritable();
    private MapWritable hMap = new MapWritable();
    private Text totalCount = new Text("*");
    private double Theta;
      
    @Override
    protected void setup(Context context) throws IOException, InterruptedException {
        Configuration conf = context.getConfiguration();
        Theta = Double.parseDouble(conf.get("Theta"));
    }


    public void reduce(Text key, Iterable<MapWritable> values,
            Context context
            ) throws IOException, InterruptedException {

      MapWritable currMap = new MapWritable();
      IntWritable currC;
      int total = 0;

      for (MapWritable map : values) {
        total += ((IntWritable) map.remove(totalCount)).get();
        for (Map.Entry<Writable, Writable> entry : map.entrySet()){
          if(currMap.containsKey(entry.getKey())){
            currC = (IntWritable) currMap.get(entry.getKey());
            currC.set(currC.get() + ((IntWritable) entry.getValue()).get());
            currMap.put(entry.getKey(), currC);
          }
          else{
            currMap.put(entry.getKey(), new IntWritable(((IntWritable) entry.getValue()).get()));
          }
        }
      }
      currMap.put(totalCount, new IntWritable(total));
      hMap.put(new Text(key), currMap);
    }

    @Override
    protected void cleanup(Context context) throws IOException, InterruptedException {
      MapWritable currMap;
      Text inital;
      double total;
      double c;
      double ratio;

      for (Map.Entry<Writable, Writable> entry : hMap.entrySet()) {
        currMap = (MapWritable) entry.getValue();
        inital = (Text) entry.getKey();
        total = ((IntWritable) currMap.remove(totalCount)).get();

        for(Map.Entry<Writable, Writable> subEntry : currMap.entrySet()){
          
          c = ((IntWritable) subEntry.getValue()).get();
          ratio = c/total;

          if(Theta <= ratio){
            finalNgram.set(inital.toString().concat(subEntry.getKey().toString()));
            finalRatio.set(ratio);
            context.write(finalNgram, finalRatio);
          }
        }
      }
    }
  }

  public static void main(String[] args) throws Exception {
      Configuration conf = new Configuration();
      conf.set("N", args[2]); //set N
      conf.set("Theta", args[3]); //set Theta
      Job job = Job.getInstance(conf, "ngram initial rf");
      job.setJarByClass(NgramInitialRF.class);
      job.setMapperClass(TokenizerMapper.class);
      //job.setCombinerClass(IntSumReducer.class);
      job.setReducerClass(IntSumReducer.class);
      job.setMapOutputKeyClass(Text.class);
      job.setMapOutputValueClass(MapWritable.class);
      job.setOutputKeyClass(Text.class);
      job.setOutputValueClass(DoubleWritable.class);
      FileInputFormat.addInputPath(job, new Path(args[0]));
      FileOutputFormat.setOutputPath(job, new Path(args[1]));
      System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}