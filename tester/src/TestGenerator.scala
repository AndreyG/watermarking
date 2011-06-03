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
  *     args(0) -- config file name
  *     args(1) -- out file name (default value defined in script "generate-tests.sh" is "test-run.sh")
  */
  def main(args: Array[String]) {
    assert(args.size == 2)
    val attrs = new common.Attributes(new File(args(0)))

    val inputDir    = new DirWrapper(new File(attrs("input-dir")))
    val outputDir   = new DirWrapper(new File(attrs("output-dir")), toMake=true)
   
    val out = new StreamWrapper(new PrintStream(new FileOutputStream(new File(args(1)))))

    val noiseRange = genRange(parseDouble(attrs("noise-lower-bound")),
                              parseDouble(attrs("noise-upper-bound")),
                              parseDouble(attrs("noise-step")))

    for (inGraphDir <- inputDir.subdirs(matchPattern(attrs("graph-name")))) {
      val outGraphDir = new DirWrapper(outputDir, inGraphDir.name)
      for (inFactorizationDir <- inGraphDir.subdirs(matchPattern(attrs("factorization-name")))) {
        val outFactorizationDir = new DirWrapper(outGraphDir, inFactorizationDir.name)
        var first = true
        for (inEmbeddingDir <- inFactorizationDir.subdirs(matchPattern(attrs("embedding-name")))) {
          val outDir = new DirWrapper(outFactorizationDir, inEmbeddingDir.name)
          val resultDir = new DirWrapper(outDir, "result")
          val logDir = new DirWrapper(outDir, "log")
  
          for (noise <- noiseRange) {
			val noiseDir = new DirWrapper(resultDir, format("noise-%.3f", noise))
			for (attempt <- 0 until parseInt(attrs("attempts-num"))) {
			  new DirWrapper(noiseDir, "attempt-" + attempt)
			}
          }

          val factorizationDumpFile = outFactorizationDir / "factorization.params"

          val inputGraph            = inGraphDir            /   "input-graph.txt" 
          val factorizationConfig   = inFactorizationDir    /   "factorization.conf" 
          val embeddingConfig       = inEmbeddingDir        /   "embedding.conf" 

          def checkFileExistance(file: File, message: String) {
            assert(file.exists, println(message + " [" + file.getAbsolutePath + "]"))
          }

          checkFileExistance(inputGraph,            "there is no file with input graph!")
          checkFileExistance(factorizationConfig,   "there is no file with factorization config")          
          checkFileExistance(embeddingConfig,       "there is no file with message embedding config")

          import common.details.{feed, endl}

          out << 
              "bin/watermarking" << feed << 
              "      --input-graph "        << inputGraph                                               << feed << 
              "      --dump-exists "        << (if (factorizationDumpFile.exists) "true" else "false")  << feed <<
              "      --factorization-dump " << factorizationDumpFile                                    << feed <<
              "      --factorization "      << factorizationConfig                                      << feed << 
              "      --embedding "          << embeddingConfig                                          << feed << 
              "      --result-dir "         << resultDir                                                << feed <<
              "      --noise-lower-bound "  << attrs("noise-lower-bound")                               << feed <<
              "      --noise-upper-bound "  << attrs("noise-upper-bound")                               << feed <<
              "      --noise-step "         << attrs("noise-step")                                      << feed <<
              "      --attempts-num "       << attrs("attempts-num")                                    << feed <<
              "      --watermarked-graph "  << (resultDir / "modified-graph.txt")                       << feed <<
              "      --statistics-file "    << (resultDir / "statistics.txt")                           << feed <<
              "      1> " << (logDir / "out.txt")                                                       << feed <<
              "      2> " << (logDir / "err.txt")                                                       << endl

          first = false
        }
      }
    }
  }
}		      
