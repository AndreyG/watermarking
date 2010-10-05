import java.io.File
import scala.util.Sorting.quickSort

object Common {
  val inputDir = new File("input")

  def isDir(file: File) = file.isDirectory

  def dirs(file: File, filter: (File => Boolean)): Array[File] = {
    assert(isDir(file))
    val res = file.listFiles.filter(file => isDir(file) && filter(file))
    quickSort(res)(new Ordering[File] {
      override def compare(x: File, y: File) = x.getName compare y.getName
    })
    res
  }

  def dirs(file: File): Array[File] = dirs(file, _ => true)

  def nameStarts(str: String): (File => Boolean) = _.getName.startsWith(str)

  def matchPattern(s: String): (File => Boolean) = dir => s.r.findPrefixOf(dir.getName).isDefined

}
