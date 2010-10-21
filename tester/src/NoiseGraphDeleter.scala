import java.io.File
import Common._

object NoiseGraphDeleter {
  def main(args: Array[String]) {
    assert(args.size == 1)
    for (graphDir <- dirs(inputDir, matchPattern(args(0)))) {
      for (factorizationDir <- dirs(graphDir)) {
		for (embeddingDir <- dirs(factorizationDir, nameStarts("alpha"))) {
		  for (timeDir <- dirs(embeddingDir)) {
			for (noiseDir <- dirs(timeDir, nameStarts("noise"))) {
			  for (attemptDir <- dirs(noiseDir, nameStarts("attempt"))) {
				val f = new File(attemptDir, "noised_graph.txt")
				if (f.exists)
				  f.delete()
			  }
			}
		  }
		}
	  }
	}
  }
}
