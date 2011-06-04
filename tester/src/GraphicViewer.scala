import scala.io.Source
import java.io.File
import java.lang.Double.parseDouble
import java.lang.{Double => JDouble}
import java.util.ArrayList
import javax.swing.{SwingUtilities, JFrame}

import common._
import common.Util.matchPattern

object GraphicViewer {
  def main(args: Array[String]) {
    assert(args.size == 1)
    val attrs = new common.Attributes(new File(args(0)))

    val outputDir = new DirWrapper(new File(attrs("output-dir")))
    val graphDir = new DirWrapper(outputDir, attrs("graph-name"))

    val data = for {
      factorizationDir <- graphDir.subdirs(matchPattern(attrs("factorization-name")));
      embeddingDir <- factorizationDir.subdirs(matchPattern(attrs("embedding-name")))
      resultDir = new DirWrapper(embeddingDir, "result");
      pairs = for {
        noiseDir <- resultDir.subdirs(matchPattern("noise-*")) 
        noise = {
          val s = noiseDir.name
          parseDouble(s.substring(s.length - 5, s.length))              
        }
        (attemptsNum, percent) = {
          var per = 0.0
          var atn = 0
          for (attemptDir <- noiseDir.subdirs(matchPattern("attempt-*"))) {
            val messageFile = attemptDir / "message.txt"
            if (messageFile.exists) {
                val source = Source.fromFile(messageFile) 
                val lines = source.getLines.toArray
                if (lines.size == 4) {
                  atn += 1

                  def cut(s: String) = s.substring(19, s.length)

                  def percentOfCommon(s1: String, s2: String) = {
                    assert(s1.length == s2.length)

                    val spacesNum = s1.count(_ == ' ')
                    val commonNum = (s1 zip s2).count(t => (t._1 == t._2))

                    (commonNum - spacesNum) * 1.0 / (s1.length - spacesNum)
                  }

                  per += percentOfCommon(cut(lines(0)), cut(lines(2)))
                }
                source.close()
            }
          }
          (atn, per)
        }
        if (attemptsNum > 0)
      } yield (noise, percent / attemptsNum)
    } yield (factorizationDir.name, pairs)

    val arguments: ArrayList[ArrayList[JDouble]] = new ArrayList
    val values: ArrayList[ArrayList[JDouble]] = new ArrayList
    val titles: ArrayList[String] = new ArrayList
    for ((title, pairs) <- data) {
      titles.add(title)
      val args = new ArrayList[JDouble]
      val vals = new ArrayList[JDouble]
      for ((a, v) <- pairs) {
        args.add(a)
        vals.add(v)
      }
      arguments.add(args)
      values.add(vals)
    }
	SwingUtilities.invokeLater(new Runnable() {
	  override def run() {
		val frame = new JFrame() {
		  getContentPane.add(new GraphicPanel(arguments, values, titles, "noise", "success percent"))
		  pack()
		}
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setSize(800, 600)
		frame.setVisible(true)
	  }
	})
  }
}
