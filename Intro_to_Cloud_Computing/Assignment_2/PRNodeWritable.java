import java.io.IOException;
import java.util.ArrayList;
import java.io.DataInput;
import java.io.DataOutput;
import org.apache.hadoop.io.Writable;

public class PRNodeWritable implements Writable {
  private int nodeID;
  private ArrayList<Integer> adjacencyList;
  private double pageRank;

  public PRNodeWritable(){

  }

  public PRNodeWritable(int id){
    this.nodeID = id;
    this.adjacencyList = new ArrayList<Integer>();
    this.pageRank = 0.0;
  }

  public void setPR(double pr){
    this.pageRank = pr;
  }

  public double getPR(){
    return this.pageRank;
  }

  public String getNodeID(){
    return String.valueOf(this.nodeID);
  }

  public void appendAL(Integer n){
    this.adjacencyList.add(n);
  }

  public ArrayList<Integer> getAL(){
    return this.adjacencyList;
  }

  public void setAL(ArrayList<Integer> l){
    this.adjacencyList = l;
  }

  public String getALasString(){
    String res = "";
    for(Integer n : adjacencyList){
      res += n + " ";
    }
    return res;
  }

  public String toString(){
    return this.getNodeID() + "_" + Double.toString(pageRank) + "_" + this.getALasString();
  }

  public static PRNodeWritable fromString(String s){
    String[] tokens = s.split("_");
    PRNodeWritable curr = new PRNodeWritable(Integer.parseInt(tokens[0]));
    curr.setPR(Double.parseDouble(tokens[1]));
    String[] neighbors = tokens[2].split("\\D");
    for(String neighbor : neighbors){
      curr.appendAL(Integer.parseInt(neighbor));
    }
    return curr;
  }

  @Override
  public void write(DataOutput out) throws IOException {
    out.writeInt(this.nodeID);
    out.writeDouble(this.pageRank);
    out.writeInt(this.adjacencyList.size());
    for(Integer n :this.adjacencyList){
      out.writeInt(n);
    }
  }

  @Override
  public void readFields(DataInput in) throws IOException {
    this.nodeID = in.readInt();
    this.pageRank = in.readDouble();
    int l = in.readInt();
    this.adjacencyList = new ArrayList<Integer>();
    for(int i = 0; i < l; i++){
      this.appendAL(in.readInt());
    }
  }  

  public static PRNodeWritable read(DataInput in) throws IOException {
    PRNodeWritable p = new PRNodeWritable();
    p.readFields(in);
    return p;
  }
}
