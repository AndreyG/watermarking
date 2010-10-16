import java.io.{File, PrintStream, FileOutputStream}
import scala.io.Source
import java.lang.Double.parseDouble
import Common._

object TestChecker {
  def main(args: Array[String]) {
    assert(args.size == 1)
    for (graphDir <- dirs(inputDir, matchPattern(args(0)))) {
      for (factorizationDir <- dirs(graphDir)) {
	for (embeddingDir <- dirs(factorizationDir, nameStarts("alpha"))) {
	  for (timeDir <- dirs(embeddingDir)) {
	    for (noiseDir <- dirs(timeDir, nameStarts("noise"))) {
	      val noise = {
		val x = parseDouble(noiseDir.getName.substring(6))
		format("%f", x)
	      }			    
	      val name = factorizationDir.getName + "/" + graphDir.getName + "/" + embeddingDir.getName + "/noise-" + noise
	      var attemptsNum = 0
	      var successesNum = 0
	      for (attemptDir <- dirs(noiseDir, nameStarts("attempt"))) {
		val f = new File(attemptDir, "message.txt")
		if (f.exists) {
		  attemptsNum += 1
		  val lines = Source.fromFile(f).getLines.toArray
		  if ((lines.length == 4) && (lines(1) == lines(3)))
		    successesNum += 1
		}
	      }
	      println(format("%s: (%d/%d)", name, successesNum, attemptsNum))
	    }
	  }
	}
      }
    }
  }
}
