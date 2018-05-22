/// QueueFamilyIndices
struct QueueFamilyIndices {

	/// Initial number of graphics families is -1
	int graphicsFamily = -1;

	/// isComplete returns true is there are one or more graphicsFamilies
	bool isComplete() {
		return graphicsFamily >= 0;
	}
};