import scala.io.Source
import java.io.File
import java.lang.Double.parseDouble

import common._
import common.Util.matchPattern

object TestChecker {
  def main(args: Array[String]) {
    assert(args.size == 2)
    val attrs = new common.Attributes(new File(args(0)))

    val outputDir = new DirWrapper(new File(attrs("output-dir")))
    val noiseLowerBound = parseDouble(attrs("noise-lower-bound"))
    val noiseUpperBound = parseDouble(attrs("noise-upper-bound"))

    val out = new StreamWrapper(System.out)

    for (graphDir <- outputDir.subdirs(matchPattern(attrs("graph-name")))) {
      out << details.black << "[" << graphDir.name << "]\n";
      for (factorizationDir <- graphDir.subdirs(matchPattern(attrs("factorization-name")))) {
        out << details.black << "   [" << factorizationDir.name << "]\n";
        for (embeddingDir <- factorizationDir.subdirs(matchPattern(attrs("embedding-name")))) {
          out << details.blue << "      " << embeddingDir.name << ":"
          val resultDir = new DirWrapper(embeddingDir, "result");
          args(1) match {
            case "show-results" => {
              out << "\n";
              for (noiseDir <- resultDir.subdirs(matchPattern("noise-*"))) {
                val noise = {
                  val s = noiseDir.name
                  parseDouble(s.substring(s.length - 5, s.length))
                }
                if ((noise >= noiseLowerBound) && (noise <= noiseUpperBound)) {
                    var attemptsNum = 0
                    var successesNum = 0
                    for (attemptDir <- noiseDir.subdirs(matchPattern("attempt-*"))) {
                      val messageFile = attemptDir / "message.txt"
                      if (messageFile.exists) {
                          val source = Source.fromFile(messageFile) 
                          val lines = source.getLines.toArray
                          if (lines.size == 4) {
                            attemptsNum += 1
                            if (lines(1) == lines(3))                
                              successesNum += 1
                          } else if (lines.size > 4) {
                            attemptsNum += 1
                          }
                          source.close()
                      } 
                    }
                    if (attemptsNum > 0) {
                        out <<  details.black   << "            " << noise << ": (" <<
                                details.green   << successesNum << 
                                details.black   << "/"          << 
                                details.red     << attemptsNum  <<
                                details.black   << ")\n"
                    }
                }
              }
            }
            case "show-angle-diff" => {
              val source = Source.fromFile(resultDir / "statistics.txt")
              val diff = source.getLines.next
              out << details.black << "     " << diff << "\n"
              source.close()
            }
          } 
        }
      }
    }
  }
}
