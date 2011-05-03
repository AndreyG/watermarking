import scala.io.Source
import java.io.File
import java.lang.Double.parseDouble
import java.lang.{Double => JDouble}
import java.util.ArrayList
import javax.swing.{SwingUtilities, JFrame}

import common._

object GraphicViewer {
  def main(args: Array[String]) {
    assert(args.size == 1)
    val attrs = new common.Attributes(new File(args(0)))

    val outputDir = new DirWrapper(new File(attrs("output-dir")))
    val graphDir = new DirWrapper(outputDir, attrs("graph-name"))

    val data = for {
      factorizationDir <- graphDir.subdirs(matchPattern(attrs("factorization-name"))) 
      embeddingDir = new DirWrapper(factorizationDir, attrs("embedding-name"))
      resultDir = new DirWrapper(embeddingDir, "result");
      pairs = for {
        noiseDir <- resultDir.subdirs(matchPattern("noise-*")) 
        noise = {
          val s = noiseDir.name
          parseDouble(s.substring(s.length - 5, s.length))              
        }
        percent = {
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
          successesNum * 1.0 / attemptsNum 
        }
      } yield (noise, percent)
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
      println(pairs.size)
    }
	SwingUtilities.invokeLater(new Runnable() {
	  override def run() {
		val frame = new JFrame() {
		  getContentPane.add(new GraphicPanel(arguments, values, titles, "noise", "success percent"))
		  pack()
		}
		frame.setSize(800, 600)
		frame.setVisible(true)
	  }
	})
  }
}
