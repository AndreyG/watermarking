import scala.io.Source
import java.io.File

import common._

object TestChecker {
  def main(args: Array[String]) {
    assert(args.size == 1)
    val attrs = new common.Attributes(new File(args(0)))

    val outputDir = new DirWrapper(new File(attrs("output-dir")))

    val out = new StreamWrapper(System.out)

    for (graphDir <- outputDir.subdirs(matchPattern(attrs("graph-name")))) {
      out << details.black << "[" << graphDir.name << "]\n";
      for (factorizationDir <- graphDir.subdirs(matchPattern(attrs("factorization-name")))) {
        out << details.black << "   [" << factorizationDir.name << "]\n";
        for (embeddingDir <- factorizationDir.subdirs(matchPattern(attrs("embedding-name")))) {
          out << details.blue << "      " << embeddingDir.name << ":\n";
          val resultDir = new DirWrapper(embeddingDir, "result");
          for (noiseDir <- resultDir.subdirs(matchPattern("noise-*"))) {
            val noise = {
              val s = noiseDir.name
              s.substring(s.length - 5, s.length)
            }
            out << details.black << "            " << noise << ": "
            var attemptsNum = 0
            var successesNum = 0
            for (attemptDir <- noiseDir.subdirs(matchPattern("attempt-*"))) {
              val messageFile = attemptDir / "message.txt"
              val lines = Source.fromFile(messageFile).getLines.toArray
              if (lines.size == 4) {
                attemptsNum += 1
                if (lines(1) == lines(3))                
                  successesNum += 1
              }
            }
            out << "(" << details.green << successesNum << details.black << "/" << 
                details.red << attemptsNum << details.black << ")\n"
          }
        }
      }
    }
  }
}
