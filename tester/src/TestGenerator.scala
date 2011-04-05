import java.io.{File, PrintStream, FileOutputStream}
import java.lang.Double.parseDouble
import java.lang.Integer.parseInt
import common.{StreamWrapper, DirWrapper, matchPattern}

object TestGenerator {
  def genRange(start: Double, finish: Double, step: Double): Iterable[Double] = {
    import scala.collection.mutable.{Buffer, ArrayBuffer}
    val res: Buffer[Double] = new ArrayBuffer
    var i = start
    while (i <= finish) {
      res += i
      i += step
    }
    res.toIterable
  }

  /*
  *   args(0) -- config file name
  */
  def main(args: Array[String]) {
    assert(args.size == 1)
    val attrs = new common.Attributes(new File(args(0)))

    val inputDir    = new DirWrapper(new File(attrs("input-dir")))
    val outputDir   = new DirWrapper(new File(attrs("output-dir")), toMake=true)
   
    val out = new StreamWrapper(new PrintStream(new FileOutputStream(new File("test-run.sh"))))

    val noiseRange = genRange(parseDouble(attrs("noise-lower-bound")),
                              parseDouble(attrs("noise-upper-bound")),
                              parseDouble(attrs("noise-step")))

    for (graphDir <- inputDir.subdirs(matchPattern(attrs("graph-name")))) {
      for (factorizationDir <- graphDir.subdirs(matchPattern(attrs("factorization-name")))) {
        var first = true
        for (embeddingDir <- factorizationDir.subdirs(matchPattern(attrs("embedding-name")))) {
          val outDir = new DirWrapper(
            new DirWrapper(
              new DirWrapper(
                outputDir,
                graphDir.name
              ),
              factorizationDir.name
            ),
            embeddingDir.name
          )
  
          for (noise <- noiseRange) {
			val noiseDir = new DirWrapper(outDir, "noise-" + noise)
			for (attempt <- 0 until parseInt(attrs("attempts-num"))) {
			  new DirWrapper(noiseDir, "attempt-" + attempt)
			}
          }

          val resultDir = new DirWrapper(outDir, "result")
          val logDir = new DirWrapper(outDir, "log")

          import common.details.{feed, endl}

          out << 
              "bin/watermarking" << feed << 
              "      --input-graph " << (graphDir / "input-graph.txt") << feed << 
              "      --factorization " << {
                                val cacheFile = factorizationDir / "factorization.params"
                                if (!first || cacheFile.exists)
                                  cacheFile
                                else
                                  factorizationDir / "factorization.conf"
                              } << feed << 
              "      --embedding " << (embeddingDir / "embedding.conf") << feed << 
              "      --noise-lower-bound " << attrs("noise-lower-bound") << feed <<
              "      --noise-upper-bound " << attrs("noise-upper-bound") << feed <<
              "      --noise-step " << attrs("noise-step") << feed <<
              "      --watermarked-graph-name " << (resultDir / "modified-graph.txt") << feed <<
              "      --statistics-file-name " << (resultDir / "statistics.txt") << feed <<
              "      1> " << (logDir / "err.txt") << feed <<
              "      2> " << (logDir / "out.txt") << endl

          first = false
        }
      }
    }
  }
}		      
