import java.io.{File, PrintStream, FileOutputStream}
import java.lang.Double.parseDouble
import Common._

object TestGenerator {
  val out = new PrintStream(new FileOutputStream(new File("test-run.sh")))

  def genRange(start: Double, finish: Double, step: Double): Array[String] = {
    import scala.collection.mutable.ArrayBuffer
    val res: ArrayBuffer[String] = new ArrayBuffer
    var i = start
    while (i <= finish) {
      res += i.toString
      i += step
    }
    res.toArray
  }

  def genNameByTime() = {
    import java.util.Calendar._

    val calendar = getInstance
    
    def extend(value: Int) = {
      val s: String = calendar.get(value).toString
      if (s.length == 1) "0" + s else s
    }
 
    val date = extend(DAY_OF_MONTH) + "-" + extend(MONTH) + "-" + extend(YEAR)
    val time = extend(HOUR_OF_DAY) + "-" + extend(MINUTE)
    date + "__" + time
  }

  def mkdirIfNotExists(root: File, name: String): File = {
    val res = new File(root, name)
    if (!res.exists)
      res.mkdir()
    res
  }
  
  def main(params: Array[String]) {
    assert(params.size == 5)
    val range = genRange(parseDouble(params(2)), parseDouble(params(3)), parseDouble(params(4)))
    val args = new Array[String](7 + 1 + range.size)
    Array.copy(range, 0, args, args.size - range.size, range.size)
    args(0) = "bin/watermarking"
    val attemptsNum = java.lang.Integer.parseInt(params(0))
    args(6) = params(0)
    args(7) = range.size.toString
    val outDirName = genNameByTime()
    for (graphDir <- dirs(inputDir, matchPattern(params(1)))) {
      args(1) = graphDir.getAbsolutePath + "/input-graph.txt" 
      for (factorizationDir <- dirs(graphDir)) {
		var first = true
		args(3) = factorizationDir.getAbsolutePath
		for (embeddingDir <- dirs(factorizationDir, nameStarts("alpha"))) {
		  args(2) = {
			val factorizationParams = new File(factorizationDir, "factorization.params")
			if (factorizationParams.exists() || !first) {
			  "dump" 
			} else {
			  factorizationDir.getAbsolutePath + "/factorization.conf"
			}
		  }
		  first = false
		  args(4) = embeddingDir.getAbsolutePath + "/embedding.conf"
 		  val outDir = mkdirIfNotExists(embeddingDir, outDirName)
		  for (noise <- range) {
			val noiseDir = mkdirIfNotExists(outDir, "noise-" + noise)
			for (attempt <- 0 until attemptsNum) {
			  mkdirIfNotExists(noiseDir, "attempt-" + attempt)
			}
		  }
		  args(5) = outDir.getAbsolutePath
		  val logDir = mkdirIfNotExists(outDir, "log")
		  val errStream = logDir.getAbsolutePath + "/err.txt"
		  val outStream = logDir.getAbsolutePath + "/out.txt" 
		  for (arg <- args) {
			out.print(arg + " ")
		  }
		  out.print(" 1> " + outStream)
		  out.print(" 2> " + errStream)
		  out.println()
		}
      }
    }
  }
}
