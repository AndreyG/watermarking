import java.io.{File, PrintStream, FileOutputStream}
import scala.io.Source

import Common._

object TestChecker {
  def main(args: Array[String]) {
    assert(args.size == 1)
    for (graphDir <- dirs(inputDir, matchPattern(args(0)))) {
      for (factorizationDir <- dirs(graphDir)) {
	for (embeddingDir <- dirs(factorizationDir, nameStarts("alpha"))) {
	  for (timeDir <- dirs(embeddingDir)) {
	    for (noiseDir <- dirs(timeDir, nameStarts("noise"))) {
	      for (attemptDir <- dirs(noiseDir, nameStarts("attempt"))) {
		for (f <- attemptDir.listFiles; if f.getName == "message.txt") {
		  val name = f.getAbsolutePath.substring(inputDir.getAbsolutePath.length)
		  val lines = Source.fromFile(f).getLines.toArray
		  if ((lines.length == 4) && (lines(1) == lines(3)))
		    println(name + " -- Accepted")
		  else
		    println(name + " -- Fail")
		}
	      }
	    }
	  }
	}
      }
    }
  }
}
