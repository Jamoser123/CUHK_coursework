import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

public class WordCountLength {

    public static class TokenizerMapper
        extends Mapper<Object, Text, Text, IntWritable>{

        private Text wordLength = new Text();
        private IntWritable counter = new IntWritable();
        private HashMap<Integer, Integer> hMap = new HashMap<>();

        public void map(Object key, Text value, Context context
                ) throws IOException, InterruptedException {
            StringTokenizer itr = new StringTokenizer(value.toString());

            while (itr.hasMoreTokens()) {
                String token = itr.nextToken();
                int len = token.length();

                if (hMap.containsKey(len)){
                        int currC = hMap.get(len);
                        hMap.put(len, currC + 1);
                }
                else{
                        hMap.put(len, 1);
                }
            }
        }

        @Override
        protected void cleanup(Context context) throws IOException, InterruptedException {
                for (Map.Entry<Integer, Integer> entry : hMap.entrySet()) {
                        wordLength.set(entry.getKey().toString());
                        counter.set(entry.getValue());
                        context.write(wordLength, counter);
                }
        }
    }

    public static class IntSumReducer
            extends Reducer<Text,IntWritable,Text,IntWritable> {
            private IntWritable result = new IntWritable();

            public void reduce(Text key, Iterable<IntWritable> values,
                    Context context
                    ) throws IOException, InterruptedException {
                int sum = 0;
                for (IntWritable val : values) {
                    sum += val.get();
                }
                result.set(sum);
                context.write(key, result);
                    }
    }

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        Job job = Job.getInstance(conf, "word count length");
        job.setJarByClass(WordCountLength.class);
        job.setMapperClass(TokenizerMapper.class);
        job.setCombinerClass(IntSumReducer.class);
        job.setReducerClass(IntSumReducer.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(IntWritable.class);
        FileInputFormat.addInputPath(job, new Path(args[0]));
        FileOutputFormat.setOutputPath(job, new Path(args[1]));
        System.exit(job.waitForCompletion(true) ? 0 : 1);
    }
}
~                        