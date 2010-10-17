import java.util.ArrayList
import java.io.{File, FileReader, BufferedReader}
import javax.swing.{SwingUtilities, JFrame}
import java.lang.Double.parseDouble
import scala.io.Source
import scala.collection.mutable.ArrayBuffer
import scala.collection.SortedMap
import scala.collection.immutable.TreeMap

object Main {
  def isDir(file: File) = file.isDirectory

  def dirs(file: File): Array[File] = {
	assert(file.exists, "file doesn't exist: " + file.getAbsolutePath)
	assert(isDir(file), "file isn't a dir: " + file.getAbsolutePath)
    file.listFiles.filter(isDir)
  }

  def readData(input: String, embeddingPattern: String): Traversable[(String, Traversable[(Double, Double)])] = {
	val inputDir = new File(input)
	for (factorizationDir <- dirs(inputDir);
		 embeddingDir <- dirs(factorizationDir); 
		 val eName = embeddingDir.getName;
		 if (eName.startsWith("alpha") && embeddingPattern.r.findFirstMatchIn(eName).isDefined)) yield {
		   var attempts: SortedMap[Double, (Int, Int)] = new TreeMap
		   for (timeDir <- dirs(embeddingDir)) {
			 for (noiseDir <- dirs(timeDir); if noiseDir.getName.startsWith("noise")) {
			   val noise = Math.round(parseDouble(noiseDir.getName.substring(6)) * 1e5) / 1e5
			   var (a, s) = { 
				 if (attempts contains noise)
				   attempts(noise)
				 else
				   (0, 0)
			   }
			   for (attemptDir <- dirs(noiseDir); if attemptDir.getName.startsWith("attempt")) {
				 val f = new File(attemptDir, "message.txt")
				 if (f.exists) {
				   a += 1
				   var in: BufferedReader = null
				   try {
					 in = new BufferedReader(new FileReader(f))
					 val lines: ArrayBuffer[String] = new ArrayBuffer
					 var st = in.readLine
					 while (st != null) {
					   lines += st
					   st = in.readLine
					 }
					 if ((lines.length == 4) && (lines(1) == lines(3)))
					   s += 1
				   } finally {
					 if (in != null)
					   in.close()
				   }
				 }
			   }
			   if (attempts contains noise) {
				 attempts -= noise
			   }
			   val newElem = (noise, (a, s))
			   attempts += newElem
			 }
		   }
		   val name = factorizationDir.getName + "/" + embeddingDir.getName
		   (name, for ((noise, (attemptsNum, successesNum)) <- attempts) yield (noise, successesNum * 1.0 / attemptsNum)) 
		 }
  }

  import java.lang.{Double => JDouble}

  def main(args: Array[String]) {
	assert(args.size >= 1 && args.size <= 2)
	val data = readData(args(0), if (args.size == 1) ".*" else args(1))
	val arguments: ArrayList[ArrayList[JDouble]] = new ArrayList()
	val values: ArrayList[ArrayList[JDouble]] = new ArrayList()
	val titles: ArrayList[String] = new ArrayList()
	for ((title, func) <- data) {
	  titles.add(title)
	  val noise: ArrayList[JDouble] = new ArrayList
	  val percent: ArrayList[JDouble] = new ArrayList
	  for ((arg, value) <- func) {
		noise.add(arg)
		percent.add(value)
	  }
	  arguments.add(noise)
	  values.add(percent)
	  for (i <- 1 until noise.size)
		assert(noise.get(i - 1).doubleValue < noise.get(i).doubleValue)
	}

	SwingUtilities.invokeLater(new Runnable() {
	  override def run() {
		val frame = new JFrame() {
		  getContentPane.add(new GraphicPanel(arguments, values, titles, "noise", "success percent"))
		  pack()
		}
		frame.setSize(800, 600)
		frame.setVisible(true)
	  }
	})
  }							 
}
