import java.io.{File, PrintStream, FileOutputStream}
import scala.io.Source
import java.lang.Double.parseDouble
import Common._
import scala.collection.Map
import scala.collection.immutable.{TreeMap}

object TestChecker {
  def main(args: Array[String]) {
    assert(args.size == 1)
    for (graphDir <- dirs(inputDir, matchPattern(args(0)))) {
      for (factorizationDir <- dirs(graphDir)) {
	for (embeddingDir <- dirs(factorizationDir, nameStarts("alpha"))) {
	  var attempts: Map[Double, (Int, Int)] = new TreeMap
	  for (timeDir <- dirs(embeddingDir)) {
	    for (noiseDir <- dirs(timeDir, nameStarts("noise"))) {
	      val noise = parseDouble(noiseDir.getName.substring(6))
	      var (a, s) = if (attempts contains noise) attempts(noise) else (0, 0)
	      for (attemptDir <- dirs(noiseDir, nameStarts("attempt"))) {
		val f = new File(attemptDir, "message.txt")
		if (f.exists) {
		  a += 1 
		  val lines = Source.fromFile(f).getLines.toArray
		  if ((lines.length == 4) && (lines(1) == lines(3)))
		    s += 1
		}
	      }
	      if (attempts contains noise)
		attempts -= noise
	      val newElem = (noise, (a, s))
	      attempts += newElem
	    }
	  }
	  for ((noise, (attemptsNum, successesNum)) <- attempts) { 
	    val name = factorizationDir.getName + "/" + graphDir.getName + "/" + embeddingDir.getName + "/noise-" + "%f".format(noise)
	    println(format("%s: (%d/%d)", name, successesNum, attemptsNum))
	  }
	}
      }
    }
  }
}
