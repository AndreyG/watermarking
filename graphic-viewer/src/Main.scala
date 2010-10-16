import java.util.ArrayList
import java.io.File
import javax.swing.{SwingUtilities, JFrame}
import java.lang.Double.parseDouble
import scala.io.Source
import scala.collection.SortedMap
import scala.collection.immutable.TreeMap

object Main {
  def isDir(file: File) = file.isDirectory

  def dirs(file: File): Array[File] = {
	assert(file.exists, "file doesn't exist: " + file.getAbsolutePath)
	assert(isDir(file), "file isn't a dir: " + file.getAbsolutePath)
    file.listFiles.filter(isDir)
  }

  def readData(input: String): Traversable[(String, Traversable[(Double, Double)])] = {
	val inputDir = new File(input)
	for (factorizationDir <- dirs(inputDir);
		 embeddingDir <- dirs(factorizationDir); if embeddingDir.getName.startsWith("alpha")) yield {
		   var attempts: SortedMap[Double, (Int, Int)] = new TreeMap
		   for (timeDir <- dirs(embeddingDir)) {
			 for (noiseDir <- dirs(timeDir); if noiseDir.getName.startsWith("noise")) {
			   val noise = parseDouble(noiseDir.getName.substring(6))
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
				   val lines = Source.fromFile(f).getLines.toArray
				   if ((lines.length == 4) && (lines(1) == lines(3)))
					 s += 1
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
	println(args.mkString)
	assert(args.size == 1)
	val data = readData(args(0))
	val arguments: ArrayList[ArrayList[JDouble]] = new ArrayList()
	val values: ArrayList[ArrayList[JDouble]] = new ArrayList()
	val titles: ArrayList[String] = new ArrayList()
	for ((title, func) <- data) {
	  titles.add(title)
	  val noise: ArrayList[JDouble] = new ArrayList
	  val percent: ArrayList[JDouble] = new ArrayList
	  for ((arg, value) <- func) {
		println("f(%f) = %f".format(arg, value))
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
